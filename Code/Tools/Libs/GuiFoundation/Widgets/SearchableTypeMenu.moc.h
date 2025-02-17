#pragma once

#include <Foundation/Containers/Set.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QObject>

class ezQtSearchableMenu;
class ezRTTI;
class QMenu;

class EZ_GUIFOUNDATION_DLL ezQtTypeMenu : public QObject
{
  Q_OBJECT

public:
  void FillMenu(QMenu* pMenu, const ezRTTI* pBaseType, bool bDerivedTypes, bool bSimpleMenu);

  static ezDynamicArray<ezString>* s_pRecentList;
  static bool s_bShowInDevelopmentFeatures;

  const ezRTTI* m_pLastSelectedType = nullptr;

Q_SIGNALS:
  void TypeSelected(QString sTypeName);

protected Q_SLOTS:
  void OnMenuAction();

private:
  QMenu* CreateCategoryMenu(const char* szCategory, ezMap<ezString, QMenu*>& existingMenus);
  void OnMenuAction(const ezRTTI* pRtti);

  QMenu* m_pMenu = nullptr;
  ezSet<const ezRTTI*> m_SupportedTypes;
  ezQtSearchableMenu* m_pSearchableMenu = nullptr;

  static ezString s_sLastMenuSearch;
};
