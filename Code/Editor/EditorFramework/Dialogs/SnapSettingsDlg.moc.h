#pragma once

#include <EditorFramework/ui_SnapSettingsDlg.h>
#include <Foundation/Containers/HybridArray.h>

class QAbstractButton;

class ezQtSnapSettingsDlg : public QDialog, public Ui_SnapSettingsDlg
{
  Q_OBJECT

public:
  ezQtSnapSettingsDlg(QWidget* pParent);

private Q_SLOTS:
  void on_ButtonBox_clicked(QAbstractButton* button);

private:
  struct KeyValue
  {
    EZ_DECLARE_POD_TYPE();

    const char* m_szKey;
    float m_fValue;
  };

  ezHybridArray<KeyValue, 16> m_Translation;
  ezHybridArray<KeyValue, 16> m_Rotation;
  ezHybridArray<KeyValue, 16> m_Scale;

  void QueryUI();
};
