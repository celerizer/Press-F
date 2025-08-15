#ifndef PRESS_F_QT_MAINWINDOW
#define PRESS_F_QT_MAINWINDOW

extern "C"
{
  #include "libpressf/src/emu.h"
  #include "libpressf/src/font.h"
  #include "libpressf/src/input.h"
  #include "libpressf/src/hw/system.h"
  #include "libpressf/src/hw/beeper.h"
}

#include "main.h"
#include "mainwindow.h"
#include "registerswindow.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>

#include <QApplication>
#include <QDropEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QFile>
#include <QMessageBox>
#include <QMimeData>
#include <QThread>
#include <QToolButton>

using namespace std;
using namespace std::chrono;

MainWindow::MainWindow()
{
  QGridLayout *layout = new QGridLayout;

  m_Settings = new QPfSettings("settings.ini", this);
  
  /* Setup toolbar */
  m_Toolbar = new QToolBar("File", this);

  QToolButton *ButFile = new QToolButton(this);
  ButFile->setIcon(QIcon(":/icons/sys"));
  ButFile->setPopupMode(QToolButton::InstantPopup);
  ButFile->setStyleSheet("QToolButton::menu-indicator { image: none; }");
  ButFile->setText(tr("File"));
  m_Toolbar->addWidget(ButFile);  

  QAction *ActLoadCart = new QAction(QIcon(":/icons/logo"), tr("Load Cart..."));
  connect(ActLoadCart, SIGNAL(triggered()), this, SLOT(onLoadCart()));
  ButFile->addAction(ActLoadCart);

  QAction *ActEjectCart = new QAction(QIcon(":/icons/logo"), tr("Eject Cart"));
  connect(ActEjectCart, SIGNAL(triggered()), this, SLOT(onEjectCart()));
  ButFile->addAction(ActEjectCart);

  QAction *ActReset = new QAction(QIcon(":/icons/logo"), tr("Reset"));
  connect(ActReset, SIGNAL(triggered()), this, SLOT(onReset()));
  ButFile->addAction(ActReset);

  QToolButton *ButOne = new QToolButton(this);
  ButOne->setText("1 / TIME");
  connect(ButOne, SIGNAL(clicked()), this, SLOT(onButton1()));
  m_Toolbar->addWidget(ButOne);

  QToolButton *ButTwo = new QToolButton(this);
  ButTwo->setText("2 / MODE");
  connect(ButTwo, SIGNAL(clicked()), this, SLOT(onButton2()));
  m_Toolbar->addWidget(ButTwo);

  QToolButton *ButThree = new QToolButton(this);
  ButThree->setText("3 / HOLD");
  connect(ButThree, SIGNAL(clicked()), this, SLOT(onButton3()));
  m_Toolbar->addWidget(ButThree);

  QToolButton *ButFour = new QToolButton(this);
  ButFour->setText("4 / START");
  connect(ButFour, SIGNAL(clicked()), this, SLOT(onButton4()));
  m_Toolbar->addWidget(ButFour);

  QToolButton *ButSettings = new QToolButton(this);
  ButSettings->setText("Settings");
  connect(ButSettings, SIGNAL(clicked()), this, SLOT(onSettings()));
  m_Toolbar->addWidget(ButSettings);

  QToolButton *ButDebugger = new QToolButton(this);
  ButDebugger->setText("Debugger");
  connect(ButDebugger, SIGNAL(clicked()), this, SLOT(onDebugger()));
  m_Toolbar->addWidget(ButDebugger);

  /* Setup window properties */
  setWindowTitle("Press F");
  setWindowIcon(QIcon(":/icons/logo"));

  /* Init sound */
  QAudioFormat Format;
  Format.setSampleRate(PF_SOUND_FREQUENCY);
  Format.setChannelCount(2);
  Format.setSampleType(QAudioFormat::SignedInt);
  Format.setSampleSize(16);
  Format.setCodec("audio/pcm");

  setAcceptDrops(true);
  resize(800, 600);

  /* Init gamepad */
  m_Gamepads[0].setDeviceId(0);
  m_Gamepads[1].setDeviceId(1);

  /* Init audio */
  m_AudioOutput = new QAudioOutput(Format, this);
  m_AudioDevice = m_AudioOutput->start();

  pressf_init(&g_ChannelF);
  f8_system_init(&g_ChannelF, F8_SYSTEM_CHANNEL_F);

  for (unsigned i = 0; i < g_ChannelF.f8device_count; i++)
    printf("Device %u: %s: $%04X - $%04X - size %04X\n", i,
           g_ChannelF.f8devices[i].name,
           g_ChannelF.f8devices[i].start,
           g_ChannelF.f8devices[i].end,
           g_ChannelF.f8devices[i].length);

  loadBios();

  /* Set up the widget that displays emulated frames */
  m_Framebuffer = new QPfFramebuffer(this);

  /* Finalize main window layout */
  layout->addWidget(m_Toolbar, 0, 0);
  layout->addWidget(m_Framebuffer, 1, 0);
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  m_EmulationThread = QThread::create([this]{timing();});
  m_EmulationThread->start();

  m_Registers = new RegistersWindow();
}

void MainWindow::timing(void)
{
  auto next = steady_clock::now();

  while (m_Active)
  {
    /* Input */
    set_input_button(0, INPUT_TIME, m_Gamepads[0].buttonL1() || wasClicked(0));
    set_input_button(0, INPUT_MODE, m_Gamepads[0].buttonSelect() || wasClicked(1));
    set_input_button(0, INPUT_HOLD, m_Gamepads[0].buttonR1() || wasClicked(2));
    set_input_button(0, INPUT_START, m_Gamepads[0].buttonStart() || wasClicked(3));

    set_input_button(4, INPUT_RIGHT,      m_Gamepads[0].buttonRight());
    set_input_button(4, INPUT_LEFT,       m_Gamepads[0].buttonLeft());
    set_input_button(4, INPUT_BACK,       m_Gamepads[0].buttonDown());
    set_input_button(4, INPUT_FORWARD,    m_Gamepads[0].buttonUp());
    set_input_button(4, INPUT_ROTATE_CCW, m_Gamepads[0].buttonX());
    set_input_button(4, INPUT_ROTATE_CW,  m_Gamepads[0].buttonB());
    set_input_button(4, INPUT_PULL,       m_Gamepads[0].buttonY());
    set_input_button(4, INPUT_PUSH,       m_Gamepads[0].buttonA());

    set_input_button(1, INPUT_RIGHT,      m_Gamepads[0].buttonRight());
    set_input_button(1, INPUT_LEFT,       m_Gamepads[0].buttonLeft());
    set_input_button(1, INPUT_BACK,       m_Gamepads[0].buttonDown());
    set_input_button(1, INPUT_FORWARD,    m_Gamepads[0].buttonUp());
    set_input_button(1, INPUT_ROTATE_CCW, m_Gamepads[0].buttonX());
    set_input_button(1, INPUT_ROTATE_CW,  m_Gamepads[0].buttonB());
    set_input_button(1, INPUT_PULL,       m_Gamepads[0].buttonY());
    set_input_button(1, INPUT_PUSH,       m_Gamepads[0].buttonA());

    /* Emulation loop */
    if (m_BiosLoaded)
      pressf_run(&g_ChannelF);

    /* Video */
    m_Framebuffer->update();

    /* Audio */
    for (unsigned i = 0; i < g_ChannelF.f8device_count; i++)
    {
      auto device = &g_ChannelF.f8devices[i];

      if (device->type == F8_DEVICE_BEEPER)
      {
        if (m_AudioDevice)
        {
          m_AudioBuffer.append(reinterpret_cast<const char*>(
            reinterpret_cast<f8_beeper_t*>(device->device)->samples),
            PF_SOUND_SAMPLES * 4);
          if (m_AudioBuffer.size() > PF_SOUND_SAMPLES * 4 * 2)
          {
            m_AudioDevice->write(m_AudioBuffer.data(), PF_SOUND_SAMPLES * 4);
            m_AudioBuffer.remove(0, static_cast<int>(PF_SOUND_SAMPLES * 4));
          }
        }
        break;
      }
    }

    auto time = static_cast<unsigned>(1000000000 / 60.0);

    /* Wake up right before next frames is needed */
    this_thread::sleep_until(next - milliseconds(2));

    /* Waitloop until exact timing */
    while (steady_clock::now() < next);

    next += nanoseconds(time);
  }
}

void MainWindow::onEjectCart()
{
  auto dummy = reinterpret_cast<u8*>(calloc(0x0400, 1));

  for (unsigned i = 0; i < 0x1800; i += 0x0400)
    f8_write(&g_ChannelF, 0x0800 + i, dummy, sizeof(dummy));
}

void MainWindow::loadBios(void)
{
  QFile bios_a_file(g_BiosPath + "/sl31253.bin");
  QFile bios_b_file(g_BiosPath + "/sl31254.bin");

  if (!bios_a_file.open(QIODevice::ReadOnly) ||
      !bios_b_file.open(QIODevice::ReadOnly))
  {
    QMessageBox::warning(this, "Press F",
      tr("Could not open BIOS files. Please check your settings."));
    m_BiosLoaded = false;
  }
  else
  {
    QByteArray bios_a_data = bios_a_file.read(1024);
    QByteArray bios_b_data = bios_b_file.read(1024);

    if (bios_a_data.size() != 1024 || bios_b_data.size() != 1024)
    {
      QMessageBox::warning(this, "Press F",
        tr("BIOS files are of incorrect size. Please check your settings."));
      m_BiosLoaded = false;
    }

    f8_write(&g_ChannelF, 0x0000, reinterpret_cast<const u8*>(bios_a_data.data()), 1024);
    f8_write(&g_ChannelF, 0x0400, reinterpret_cast<const u8*>(bios_b_data.data()), 1024);

    m_BiosLoaded = true;
  }
}

bool MainWindow::loadCartridge(QString Filename)
{
  QFile Rom(Filename);

  if (!Rom.open(QIODevice::ReadOnly))
    return false;
  else
  {
    for (unsigned i = 0; i < Rom.size(); i += 0x0400)
    {
      u8 temp[0x0400];

      Rom.read(reinterpret_cast<char*>(temp), sizeof(temp));
      f8_write(&g_ChannelF, 0x0800 + i, temp, sizeof(temp));
    }

    /*
      Update the window title to something friendly looking, ie:
      "/home/user/Bowling (1978) (Fairchild-21).chf" -> "Bowling"
    */
    setWindowTitle(tr("Press F - ") +
                   Filename.split('/').last().split('(').first());

    pressf_reset(&g_ChannelF);
    return true;
  }
}

void MainWindow::onButton1()
{
  m_ClickedButtons[0] = true;
}

void MainWindow::onButton2()
{
  m_ClickedButtons[1] = true;
}

void MainWindow::onButton3()
{
  m_ClickedButtons[2] = true;
}

void MainWindow::onButton4()
{
  m_ClickedButtons[3] = true;
}

void MainWindow::onDebugger()
{
  if (!m_Registers->isVisible())
    m_Registers->show();
}

void MainWindow::onLoadCart()
{
  if (!m_BiosLoaded)
    loadBios();
  loadCartridge(QFileDialog::getOpenFileName(this,
    tr("Load Cartridge"), "./games", tr("Channel F ROM Files (*.bin *.chf)")));
}

void MainWindow::onReset()
{
  if (!m_BiosLoaded)
    loadBios();
  pressf_reset(&g_ChannelF);
}

void MainWindow::onSettings()
{
  QWidget *settingsWidget = new QWidget(this);

  QVBoxLayout *settingsLayout = new QVBoxLayout(settingsWidget);
  settingsLayout->addWidget(m_Settings);
  settingsWidget->setWindowTitle("Settings");
  settingsWidget->setWindowFlags(Qt::Window);
  settingsWidget->resize(400, 300);
  settingsWidget->show();
}

bool MainWindow::wasClicked(unsigned index)
{
  if (index > sizeof(m_ClickedButtons))
    return false;
  else
  {
    bool val = m_ClickedButtons[index];
    m_ClickedButtons[index] = false;

    return val;
  }
};

void MainWindow::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)
  QApplication::quit();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
  const QMimeData* data = event->mimeData();

  if (data->hasUrls())
  {
    event->acceptProposedAction();
    loadCartridge(data->urls().at(0).toLocalFile());
  }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  m_Framebuffer->setScale(event->size());
}

#endif
