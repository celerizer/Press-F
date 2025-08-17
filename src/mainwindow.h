#ifndef PRESS_F_QT_MAINWINDOW_H
#define PRESS_F_QT_MAINWINDOW_H

#include <QAudioOutput>
#include <QBuffer>
#include <QGamepad>
#include <QLabel>
#include <QToolBar>

#include "framebuffer_widget.h"
#include "registerswindow.h"
#include "settings.h"

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow();

private:
  QGamepad m_Gamepads[2];
  QThread *m_EmulationThread;
  QToolBar *m_Toolbar;

  QPfFramebuffer *m_Framebuffer;
  RegistersWindow *m_Registers;
  QPfSettings *m_Settings;

  /* Audio stuff (TODO: Move to own file/class) */
  QAudioOutput *m_AudioOutput;
  QByteArray m_AudioBuffer;
  QIODevice *m_AudioDevice;

  QToolButton *m_LedIcon;

  bool m_Active = true;
  bool m_BiosLoaded = false;
  bool m_ClickedButtons[4];

  void loadBios(void);

  bool loadCartridge(QString Filename);

  void timing(void);

private slots:
  void onDebugger(void);
  void onEjectCart(void);
  void onLoadCart(void);
  void onReset(void);
  void onSettings(void);

protected:
  void closeEvent(QCloseEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
};

#endif
