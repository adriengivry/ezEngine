#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/DdlSerializer.h>
#include <ToolsFoundation/Document/DocumentTasks.h>

ezSaveDocumentTask::ezSaveDocumentTask()
{
  ConfigureTask("ezSaveDocumentTask", ezTaskNesting::Maybe);
}

ezSaveDocumentTask::~ezSaveDocumentTask() = default;

void ezSaveDocumentTask::Execute()
{
  ezAbstractGraphDdlSerializer::WriteDocument(file, &headerGraph, &objectGraph, &typesGraph, false);

  if (file.Close() == EZ_FAILURE)
  {
    m_document->m_LastSaveResult = ezStatus(ezFmt("Unable to open file '{0}' for writing!", m_document->m_sDocumentPath));
  }
  else
  {
    m_document->m_LastSaveResult = ezStatus(EZ_SUCCESS);
  }
}

ezAfterSaveDocumentTask::ezAfterSaveDocumentTask()
{
  ConfigureTask("ezAfterSaveDocumentTask", ezTaskNesting::Maybe);
}

ezAfterSaveDocumentTask::~ezAfterSaveDocumentTask() = default;

void ezAfterSaveDocumentTask::Execute()
{
  if (m_document->m_LastSaveResult.Succeeded())
  {
    ezDocumentEvent e;
    e.m_pDocument = m_document;
    e.m_Type = ezDocumentEvent::Type::DocumentSaved;
    m_document->m_EventsOne.Broadcast(e);
    m_document->s_EventsAny.Broadcast(e);

    m_document->SetModified(false);

    // after saving once, this information is pointless
    m_document->m_uiUnknownObjectTypeInstances = 0;
    m_document->m_UnknownObjectTypes.Clear();
  }

  if (m_document->m_LastSaveResult.Succeeded())
  {
    m_document->InternalAfterSaveDocument();
  }
  if (m_callback.IsValid())
  {
    m_callback(m_document, m_document->m_LastSaveResult);
  }
  m_document->m_ActiveSaveTask.Invalidate();
}
