using DevExpress.Mvvm;

namespace TOS_TW_TOOL.Models
{
    public class StatusInfoModel : ViewModelBase
    {
        public int HP
        {
            get => GetProperty(() => HP);
            set => SetProperty(() => HP, value);
        }

        public int MaxHP
        {
            get => GetProperty(() => MaxHP);
            set => SetProperty(() => MaxHP, value);
        }

        public int SP
        {
            get => GetProperty(() => SP);
            set => SetProperty(() => SP, value);
        }

        public int MaxSP
        {
            get => GetProperty(() => MaxSP);
            set => SetProperty(() => MaxSP, value);
        }
    }
}
