using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Amib.Threading;
using DevExpress.Mvvm;
using DevExpress.Mvvm.DataAnnotations;
using InputInterceptorNS;
using TOS_TW_TOOL.Dll;
using TOS_TW_TOOL.Models;

namespace TOS_TW_TOOL.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        enum Status_Flag
        {
            info = 1,
            err = 0
        }

        #region public

        /// <summary>
        /// 进程信息集合
        /// </summary>
        public ObservableCollection<ProcessModel> ProcessInfoes { get; set; }

        /// <summary>
        /// 游戏进程索引
        /// </summary>
        public int GameProcessIndex
        {
            get => GetProperty(() => GameProcessIndex);
            set => SetProperty(() => GameProcessIndex, value);
        }

        /// <summary>
        /// 状态栏标志
        /// </summary>
        public string StatusFlag
        {
            get => GetProperty(() => StatusFlag);
            set => SetProperty(() => StatusFlag, value);
        }

        /// <summary>
        /// 状态栏文本
        /// </summary>
        public string StatusText
        {
            get => GetProperty(() => StatusText);
            set => SetProperty(() => StatusText, value);
        }

        /// <summary>
        /// 快捷键
        /// </summary>
        public HotKeyModel HotKey
        {
            get => GetProperty(() => HotKey);
            set => SetProperty(() => HotKey, value);
        }

        /// <summary>
        /// 角色状态信息
        /// </summary>
        public StatusInfoModel StatusInfo
        {
            get => GetProperty(() => StatusInfo);
            set => SetProperty(() => StatusInfo, value);
        }

        /// <summary>
        /// 是否找到状态栏坐标
        /// </summary>
        public bool IsNotFoundPos
        {
            get => GetProperty(() => IsNotFoundPos);
            set => SetProperty(() => IsNotFoundPos, value, () => { CanDo = !value; });
        }

        /// <summary>
        /// 是否允许执行
        /// </summary>
        public bool CanDo
        {
            get => GetProperty(() => CanDo);
            set => SetProperty(() => CanDo, value);
        }

        /// <summary>
        /// 驱动状态
        /// </summary>
        public string DriverStatus
        {
            get => GetProperty(() => DriverStatus);
            set => SetProperty(() => DriverStatus, value);
        }

        #endregion

        #region private

        /// <summary>
        /// 角色状态栏位置
        /// </summary>
        private CharacterStatusBarLoc csbLoc;

        /// <summary>
        /// 状态栏信息
        /// </summary>
        private CharacterStatusBarInfo csbInfo;

        /// <summary>
        /// 计算HP药剂CD
        /// </summary>
        private DateTime hp_cd_time = DateTime.Now;

        /// <summary>
        /// 计算SP药剂CD
        /// </summary>
        private DateTime sp_cd_time = DateTime.Now;

        /// <summary>
        /// 后台执行定时器
        /// </summary>
        private DispatcherTimer timer_ocr = new DispatcherTimer();

        /// <summary>
        /// 后台执行定时器
        /// </summary>
        private DispatcherTimer timer_line = new DispatcherTimer();

        /// <summary>
        /// 智能线程池
        /// </summary>
        private SmartThreadPool stp = new SmartThreadPool();

        /// <summary>
        /// 拦截器上下文
        /// </summary>
        private IntPtr interception_context;

        /// <summary>
        /// 用于拦截器的设备号
        /// </summary>
        private int keyboard_device;

        /// <summary>
        /// HP药水键冲程
        /// </summary>
        private Stroke stroke_hp = new Stroke();

        /// <summary>
        /// SP药水键冲程
        /// </summary>
        private Stroke stroke_sp = new Stroke();

        /// <summary>
        /// 随机数生成器
        /// </summary>
        private readonly Random rnd = new Random();

        #endregion

        #region ctor

        public MainViewModel()
        {
            // 初始化
            Init();
        }

        #endregion

        #region private

        /// <summary>
        /// 初始化
        /// </summary>
        private void Init()
        {
            // 获取高权限令牌
            using (Process p = Process.GetCurrentProcess())
            {
                p.PriorityClass = ProcessPriorityClass.High;
            }
            // 初始化拦截器
            if (CheckInterceptorDriverInstalled())
            {
                if (InputInterceptor.Initialize())
                {
                    // 获取拦截器上下文及键盘设备
                    interception_context = InputInterceptor.CreateContext();
                    var lt = InputInterceptor.GetDeviceList(interception_context);
                    keyboard_device = lt.Where(o => { return InputInterceptor.IsKeyboard(o.Device); }).FirstOrDefault().Device;

                    // 初始化进程列表
                    if (InitProcessList())
                    {
                        // 初始化成员对象
                        StatusInfo = new StatusInfoModel();
                        HotKey = new HotKeyModel();
                        csbInfo = new CharacterStatusBarInfo();
                        IsNotFoundPos = true;
                        CanDo = false;
                        // 初始化定时器
                        timer_ocr.Tick += new EventHandler(Timer_Tick_GetStatusInfosFromOCR);
                        timer_ocr.Interval = TimeSpan.FromMilliseconds(2000);
                        timer_line.Tick += new EventHandler(Timer_Tick_GetStatusInfosFromLine);
                        timer_line.Interval = TimeSpan.FromMilliseconds(2000);
                        // 初始化热键设置
                        HotKey.Key4HPEnable = false;
                        HotKey.Key4SPEnable = true;
                        HotKey.HotKeyTextBoxFocusable4HP = false;
                        HotKey.HotKeyTextBoxFocusable4SP = false;
                    }
                }
                else
                {
                    SetStatusInfo(Status_Flag.err, "Input interceptor initialization failed.");
                    MessageBox.Show("Input interceptor initialization failed.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        /// <summary>
        /// 定时器获取状态信息（Line）
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Timer_Tick_GetStatusInfosFromLine(object sender, EventArgs e)
        {
            stp.QueueWorkItem(() =>
            {
                EjiTosBot.GetStatusBarInfoFromLine(ProcessInfoes[GameProcessIndex].Pid, csbLoc, out csbInfo);

                StatusInfo.MaxHP = csbInfo.hp_max;
                StatusInfo.HP = csbInfo.hp;
                StatusInfo.MaxSP = csbInfo.sp_max;
                StatusInfo.SP = csbInfo.sp;

                // 判断HP、SP
                if (HotKey.Key4HPEnable && (!string.IsNullOrWhiteSpace(HotKey.Key4HP)) && StatusInfo.HP <= 70)
                {
                    // 判断药水CD
                    if ((DateTime.Now - hp_cd_time).TotalSeconds > 15)
                    {
                        stroke_hp.Key.State = KeyState.Down;
                        InputInterceptor.Send(interception_context, keyboard_device, ref stroke_hp, 1);
                        Thread.Sleep(rnd.Next(1, 50));
                        stroke_hp.Key.State = KeyState.Up;
                        InputInterceptor.Send(interception_context, keyboard_device, ref stroke_hp, 1);
                        // 重置CD
                        hp_cd_time = DateTime.Now;
                    }
                }
                if (HotKey.Key4SPEnable && (!string.IsNullOrWhiteSpace(HotKey.Key4SP)) && StatusInfo.SP <= 70)
                {
                    // 判断药水CD
                    if ((DateTime.Now - sp_cd_time).TotalSeconds > 15)
                    {
                        stroke_sp.Key.State = KeyState.Down;
                        InputInterceptor.Send(interception_context, keyboard_device, ref stroke_sp, 1);
                        Thread.Sleep(rnd.Next(1, 50));
                        stroke_sp.Key.State = KeyState.Up;
                        InputInterceptor.Send(interception_context, keyboard_device, ref stroke_sp, 1);
                        // 重置CD
                        sp_cd_time = DateTime.Now;
                    }
                }
            });
        }

        /// <summary>
        /// 定时获取状态信息（Ocr）
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Timer_Tick_GetStatusInfosFromOCR(object sender, EventArgs e)
        {
            stp.QueueWorkItem(() =>
            {
                EjiTosBot.GetStatusBarInfoFromOCR(ProcessInfoes[GameProcessIndex].Pid, csbLoc, out csbInfo);

                StatusInfo.MaxHP = csbInfo.hp_max;
                StatusInfo.HP = csbInfo.hp;
                StatusInfo.MaxSP = csbInfo.sp_max;
                StatusInfo.SP = csbInfo.sp;
            });
        }

        /// <summary>
        /// 判断是否安装拦截器驱动
        /// </summary>
        private bool CheckInterceptorDriverInstalled()
        {
            if (InputInterceptor.CheckDriverInstalled())
            {
                SetStatusInfo(Status_Flag.info, "Input interceptor successfully initialized.");
                DriverStatus = "Installed";
                return true;
            }
            else
            {
                DriverStatus = "Not installed";
                SetStatusInfo(Status_Flag.info, "Input interceptor not installed.");
                var msgRet = MessageBox.Show("Input interceptor not installed. Do you want to install it?", "Warning", MessageBoxButton.YesNo, MessageBoxImage.Warning);
                if (msgRet == MessageBoxResult.Yes)
                {
                    // 安装驱动
                    SetStatusInfo(Status_Flag.info, "Installing...");
                    if (InputInterceptor.InstallDriver())
                    {
                        SetStatusInfo(Status_Flag.info, "Done! Please restart your computer.");
                        MessageBox.Show("Done! Please restart your computer.", "Info", MessageBoxButton.OK, MessageBoxImage.Information);
                        return true;
                    }
                    else
                    {
                        SetStatusInfo(Status_Flag.err, "Unknown exception, installation failed.");
                        MessageBox.Show("Unknown exception, installation failed.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return false;
                    }

                }
                else
                {
                    return false;
                }
            }
        }

        /// <summary>
        /// 初始化进程列表
        /// </summary>
        private bool InitProcessList()
        {
            // 获取进程列表
            EjiTosBot.GetProcessList(out int[] pid_arr, out string[] proc_name_arr);
            ProcessInfoes = new ObservableCollection<ProcessModel>();
            for (int i = 0; i < pid_arr.Length; i++)
            {
                ProcessInfoes.Add(new ProcessModel { Index = i, Pid = pid_arr[i], ProcessName = proc_name_arr[i] });
            }
            // 筛选出游戏进程，并定位索引
            var game = ProcessInfoes.Where(x => x.ProcessName == "Client_tos.exe")
                .FirstOrDefault();
            GameProcessIndex = game == null ? -1 : game.Index;
            if (-1 == GameProcessIndex)
            {
                SetStatusInfo(Status_Flag.err, "The game process was not found.");
                GameProcessIndex = 0;
                return false;
            }
            else
            {
                SetStatusInfo(Status_Flag.info, "Client_tos.exe has been found.");
                return true;
            }
        }

        /// <summary>
        /// 设置状态栏信息
        /// </summary>
        /// <param name="flag"></param>
        /// <param name="text"></param>
        private void SetStatusInfo(Status_Flag flag, string text)
        {
            if (flag == Status_Flag.err)
            {
                StatusFlag = "Error";
                StatusText = text;
            }
            else
            {
                StatusFlag = "Info";
                StatusText = text;
            }
        }

        #endregion

        #region command

        [AsyncCommand]
        public void InitCommand(object obj)
        {
            csbLoc = new CharacterStatusBarLoc();
            EjiTosBot.GameCaptureInit(ProcessInfoes[GameProcessIndex].Pid, ref csbLoc);
            if (csbLoc.x > 0)
            {
                SetStatusInfo(Status_Flag.info, $"Location: x={csbLoc.x} y={csbLoc.y} w={csbLoc.w} h={csbLoc.h}.");
                IsNotFoundPos = false;
            }
            else
            {
                SetStatusInfo(Status_Flag.err, "Positioning failure.");
                IsNotFoundPos = true;
            }

            // 键盘Hook，用于获取热键设置
            KeyboardHook keyboardHook = new KeyboardHook((ref KeyStroke keyStroke) =>
            {
                if (HotKey.HotKeyTextBoxFocusable4HP)
                {
                    HotKey.Key4HP = keyStroke.Code.ToString();
                    stroke_hp.Key.Code = keyStroke.Code;
                    HotKey.HotKeyTextBoxFocusable4HP = false;
                }
                if (HotKey.HotKeyTextBoxFocusable4SP)
                {
                    HotKey.Key4SP = keyStroke.Code.ToString();
                    stroke_sp.Key.Code = keyStroke.Code;
                    HotKey.HotKeyTextBoxFocusable4SP = false;
                }
            });
        }

        [AsyncCommand]
        public void GetStatusInfoFromLineCommand(object obj)
        {
            var btn = obj as Button;

            if (timer_line.IsEnabled)
            {
                btn.Content = "Get status info (line)";
                timer_line.Stop();
            }
            else
            {
                btn.Content = "Running";
                timer_line.Start();
            }
        }

        [AsyncCommand]
        public void GetStatusInfoFromOcrCommand(object obj)
        {
            timer_ocr.Start();
        }

        #endregion
    }
}
