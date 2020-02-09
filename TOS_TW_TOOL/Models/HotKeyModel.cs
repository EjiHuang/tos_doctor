using DevExpress.Mvvm;
using InputInterceptorNS;

namespace TOS_TW_TOOL.Models
{
    public class HotKeyModel : ViewModelBase
    {
        /// <summary>
        /// HP药水快捷键
        /// </summary>
        public string Key4HP
        {
            get => GetProperty(() => Key4HP);
            set => SetProperty(() => Key4HP, value);
        }

        /// <summary>
        /// SP药水快捷键
        /// </summary>
        public string Key4SP
        {
            get => GetProperty(() => Key4SP);
            set => SetProperty(() => Key4SP, value);
        }

        /// <summary>
        /// 启动HP热键监控
        /// </summary>
        public bool Key4HPEnable
        {
            get => GetProperty(() => Key4HPEnable);
            set => SetProperty(() => Key4HPEnable, value);
        }

        /// <summary>
        /// 启动SP热键监控
        /// </summary>
        public bool Key4SPEnable
        {
            get => GetProperty(() => Key4SPEnable);
            set => SetProperty(() => Key4SPEnable, value);
        }

        /// <summary>
        /// HP热键框焦点
        /// </summary>
        public bool HotKeyTextBoxFocusable4HP
        {
            get => GetProperty(() => HotKeyTextBoxFocusable4HP);
            set => SetProperty(() => HotKeyTextBoxFocusable4HP, value);
        }

        /// <summary>
        /// HP热键框焦点
        /// </summary>
        public bool HotKeyTextBoxFocusable4SP
        {
            get => GetProperty(() => HotKeyTextBoxFocusable4SP);
            set => SetProperty(() => HotKeyTextBoxFocusable4SP, value);
        }
    }
}
