#pragma once

#include <EditorFramework/GUI/RawDocumentTreeModel.moc.h>
#include <QAbstractItemModel>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezSkeletonAssetDocument;

class ezQtJointAdapter : public ezQtNamedAdapter
{
  Q_OBJECT;

public:
  ezQtJointAdapter(const ezSkeletonAssetDocument* pDocument);
  ~ezQtJointAdapter();
  virtual QVariant data(const ezDocumentObject* pObject, int iRow, int iColumn, int iRole) const override;

private:
  const ezSkeletonAssetDocument* m_pDocument;
};
