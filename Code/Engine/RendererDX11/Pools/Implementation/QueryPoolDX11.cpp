#include <RendererDX11/RendererDX11PCH.h>

#include <RendererDX11/Device/DeviceDX11.h>
#include <RendererDX11/Pools/FencePoolDX11.h>
#include <RendererDX11/Pools/QueryPoolDX11.h>

ezQueryPoolDX11::ezQueryPoolDX11(ezGALDeviceDX11* pDevice)
  : m_pDevice(pDevice)
  , m_TimestampPool(pDevice->GetAllocator())
  , m_OcclusionPool(pDevice->GetAllocator())
  , m_OcclusionPredicatePool(pDevice->GetAllocator())
  , m_PendingFrames(pDevice->GetAllocator())
  , m_FreeFrames(pDevice->GetAllocator())
{
}

ezResult ezQueryPoolDX11::Initialize()
{
  EZ_SUCCEED_OR_RETURN(m_TimestampPool.Initialize(m_pDevice, D3D11_QUERY_TIMESTAMP, 2048));
  EZ_SUCCEED_OR_RETURN(m_OcclusionPool.Initialize(m_pDevice, D3D11_QUERY_OCCLUSION, 512));
  EZ_SUCCEED_OR_RETURN(m_OcclusionPredicatePool.Initialize(m_pDevice, D3D11_QUERY_OCCLUSION_PREDICATE, 512));

  m_SyncTimeDiff = ezTime::MakeZero();
  return EZ_SUCCESS;
}

void ezQueryPoolDX11::DeInitialize()
{
  m_TimestampPool.DeInitialize();
  m_OcclusionPool.DeInitialize();
  m_OcclusionPredicatePool.DeInitialize();

  for (auto& perFrameData : m_FreeFrames)
  {
    EZ_GAL_DX11_RELEASE(perFrameData.m_pDisjointTimerQuery);
  }
  for (auto& perFrameData : m_PendingFrames)
  {
    EZ_GAL_DX11_RELEASE(perFrameData.m_pDisjointTimerQuery);
  }
}

void ezQueryPoolDX11::BeginFrame()
{
  ezUInt64 uiCurrentFrame = m_pDevice->GetCurrentFrame();

  auto perFrameData = GetFreeFrame();
  perFrameData.m_uiFrameCounter = uiCurrentFrame;
  m_pDevice->GetDXImmediateContext()->Begin(perFrameData.m_pDisjointTimerQuery);
  perFrameData.m_fInvTicksPerSecond = -1.0f;
  m_PendingFrames.PushBack(perFrameData);

  for (PerFrameData& data : m_PendingFrames)
  {
    if (data.m_fInvTicksPerSecond != s_fInvalid)
      data.m_uiReadyFrames++;
  }

  // Clear out old frames
  while (m_PendingFrames[0].m_uiReadyFrames > s_uiRetainFrames)
  {
    PerFrameData& data = m_PendingFrames.PeekFront();
    data.m_hFence = {};
    data.m_fInvTicksPerSecond = s_fInvalid;
    data.m_uiReadyFrames = 0;
    data.m_uiFrameCounter = ezUInt64(-1);

    m_FreeFrames.PushBack(data);
    m_PendingFrames.PopFront();
  }
  m_uiFirstFrameIndex = m_PendingFrames[0].m_uiFrameCounter;
}

void ezQueryPoolDX11::EndFrame()
{
  {
    auto& perFrameData = m_PendingFrames.PeekBack();
    m_pDevice->GetDXImmediateContext()->End(perFrameData.m_pDisjointTimerQuery);
    perFrameData.m_hFence = m_pDevice->GetFenceQueue().GetCurrentFenceHandle();
  }

  // Get Results
  for (ezUInt32 i = 0; i < m_PendingFrames.GetCount(); i++)
  {
    auto& perFrameData = m_PendingFrames[i];
    if (m_pDevice->GetFenceQueue().GetFenceResult(perFrameData.m_hFence) != ezGALAsyncResult::Ready)
      break;

    if (perFrameData.m_fInvTicksPerSecond == s_fInvalid)
    {
      D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data = {};
      HRESULT res = m_pDevice->GetDXImmediateContext()->GetData(perFrameData.m_pDisjointTimerQuery, &data, sizeof(data), D3D11_ASYNC_GETDATA_DONOTFLUSH);
      if (res == S_OK)
      {
        if (data.Disjoint)
        {
          perFrameData.m_fInvTicksPerSecond = 0.0;
          continue;
        }

        perFrameData.m_fInvTicksPerSecond = 1.0 / (double)data.Frequency;

        if (m_bSyncTimeNeeded)
        {
          ezGALTimestampHandle hTimestamp = InsertTimestamp();
          ID3D11Query* pQuery = m_TimestampPool.GetQuery(hTimestamp);
          ezUInt64 uiTimestamp;
          while (m_pDevice->GetDXImmediateContext()->GetData(pQuery, &uiTimestamp, sizeof(uiTimestamp), 0) != S_OK)
          {
            ezThreadUtils::YieldTimeSlice();
          }

          m_SyncTimeDiff = ezTime::Now() - ezTime::MakeFromSeconds(double(uiTimestamp) * perFrameData.m_fInvTicksPerSecond);
          m_bSyncTimeNeeded = false;
        }
      }
    }
  }
}


ezQueryPoolDX11::PerFrameData ezQueryPoolDX11::GetFreeFrame()
{
  if (!m_FreeFrames.IsEmpty())
  {
    PerFrameData data = m_FreeFrames.PeekFront();
    m_FreeFrames.PopFront();
    return data;
  }

  PerFrameData perFrameData;
  D3D11_QUERY_DESC disjointQueryDesc;
  disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
  disjointQueryDesc.MiscFlags = 0;
  HRESULT res = m_pDevice->GetDXDevice()->CreateQuery(&disjointQueryDesc, &perFrameData.m_pDisjointTimerQuery);
  EZ_ASSERT_DEV(SUCCEEDED(res), "Creation of native DirectX query for disjoint query has failed!");
  EZ_IGNORE_UNUSED(res);
  return perFrameData;
}

ezGALTimestampHandle ezQueryPoolDX11::InsertTimestamp()
{
  ezGALTimestampHandle hTimestamp = m_TimestampPool.CreateQuery();
  ID3D11Query* pDXQuery = m_TimestampPool.GetQuery(hTimestamp);
  m_pDevice->GetDXImmediateContext()->End(pDXQuery);
  return hTimestamp;
}

ezEnum<ezGALAsyncResult> ezQueryPoolDX11::GetTimestampResult(ezGALTimestampHandle hTimestamp, ezTime& out_result)
{
  out_result = ezTime();
  if (hTimestamp.m_Generation < m_uiFirstFrameIndex)
  {
    // expired
    return ezGALAsyncResult::Expired;
  }

  const ezUInt32 uiFrameIndex = static_cast<ezUInt32>(hTimestamp.m_Generation - m_uiFirstFrameIndex);
  PerFrameData& pPerFrameData = m_PendingFrames[uiFrameIndex];
  // Check whether frequency and sync timer are already available for the frame of the timestamp
  if (pPerFrameData.m_fInvTicksPerSecond == s_fInvalid)
    return ezGALAsyncResult::Pending;

  ezUInt64 uiTimestamp;
  ezEnum<ezGALAsyncResult> res = m_TimestampPool.GetResult(hTimestamp, uiTimestamp);
  if (res != ezGALAsyncResult::Ready)
    return res;

  if (pPerFrameData.m_fInvTicksPerSecond == 0.0)
  {
    out_result = ezTime::MakeZero();
    return ezGALAsyncResult::Expired;
  }
  else
  {
    out_result = ezTime::MakeFromSeconds(double(uiTimestamp) * pPerFrameData.m_fInvTicksPerSecond) + m_SyncTimeDiff;
    return ezGALAsyncResult::Ready;
  }
}


ezGALPoolHandle ezQueryPoolDX11::BeginOcclusionQuery(ezEnum<ezGALQueryType> type)
{
  ezGALPoolHandle hPool;
  ID3D11Query* pQuery = nullptr;
  if (type == ezGALQueryType::NumSamplesPassed)
  {
    hPool = m_OcclusionPool.CreateQuery();
    pQuery = m_OcclusionPool.GetQuery(hPool);
  }
  else if (type == ezGALQueryType::AnySamplesPassed)
  {
    hPool = m_OcclusionPredicatePool.CreateQuery();
    pQuery = m_OcclusionPredicatePool.GetQuery(hPool);
    hPool.m_InstanceIndex |= s_uiPredicateFlag;
  }
  m_pDevice->GetDXImmediateContext()->Begin(pQuery);

  return hPool;
}


void ezQueryPoolDX11::EndOcclusionQuery(ezGALPoolHandle hPool)
{
  ID3D11Query* pQuery = nullptr;
  bool bPredicate = (hPool.m_InstanceIndex & s_uiPredicateFlag) != 0;
  hPool.m_InstanceIndex &= ~s_uiPredicateFlag;
  if (bPredicate)
  {
    pQuery = m_OcclusionPredicatePool.GetQuery(hPool);
  }
  else
  {
    pQuery = m_OcclusionPool.GetQuery(hPool);
  }
  m_pDevice->GetDXImmediateContext()->End(pQuery);
}


ezEnum<ezGALAsyncResult> ezQueryPoolDX11::GetOcclusionQueryResult(ezGALPoolHandle hPool, ezUInt64& out_uiQueryResult)
{
  out_uiQueryResult = 0;
  bool bPredicate = (hPool.m_InstanceIndex & s_uiPredicateFlag) != 0;
  hPool.m_InstanceIndex &= ~s_uiPredicateFlag;
  if (bPredicate)
  {
    ezUInt32 uiTemp;
    ezEnum<ezGALAsyncResult> res = m_OcclusionPredicatePool.GetResult(hPool, uiTemp);
    out_uiQueryResult = uiTemp;
    return res;
  }
  else
  {
    return m_OcclusionPool.GetResult(hPool, out_uiQueryResult);
  }
}

ezQueryPoolDX11::Pool::Pool(ezAllocator* pAllocator)
  : m_Queries(pAllocator)
{
}

ezResult ezQueryPoolDX11::Pool::Initialize(ezGALDeviceDX11* pDevice, D3D11_QUERY queryType, ezUInt32 uiCount)
{
  m_pDevice = pDevice;

  D3D11_QUERY_DESC timerQueryDesc;
  timerQueryDesc.Query = queryType;
  timerQueryDesc.MiscFlags = 0;

  m_Queries.SetCountUninitialized(uiCount);
  for (ezUInt32 i = 0; i < m_Queries.GetCount(); ++i)
  {
    if (FAILED(pDevice->GetDXDevice()->CreateQuery(&timerQueryDesc, &m_Queries[i])))
    {
      ezLog::Error("Creation of native DirectX query for timestamp has failed!");
      return EZ_FAILURE;
    }
  }
  return EZ_SUCCESS;
}

void ezQueryPoolDX11::Pool::DeInitialize()
{
  for (auto& timestamp : m_Queries)
  {
    EZ_GAL_DX11_RELEASE(timestamp);
  }
  m_Queries.Clear();
}

ezGALPoolHandle ezQueryPoolDX11::Pool::CreateQuery()
{
  ezUInt32 uiIndex = m_uiNextTimestamp;
  m_uiNextTimestamp = (m_uiNextTimestamp + 1) % m_Queries.GetCount();
  ezGALTimestampHandle hTimestamp = {uiIndex, m_pDevice->GetCurrentFrame()};
  return hTimestamp;
}

ID3D11Query* ezQueryPoolDX11::Pool::GetQuery(ezGALPoolHandle hPool)
{
  if (hPool.m_InstanceIndex < m_Queries.GetCount())
  {
    return m_Queries[static_cast<ezUInt32>(hPool.m_InstanceIndex)];
  }

  return nullptr;
}
