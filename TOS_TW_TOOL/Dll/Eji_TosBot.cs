using System.Runtime.InteropServices;

namespace TOS_TW_TOOL.Dll
{
    /// <summary>
    /// 角色状态栏位置信息
    /// </summary>
    public struct CharacterStatusBarLoc
    {
        public int x;
        public int y;
        public int w;
        public int h;
    };

    /// <summary>
    /// 角色状态信息
    /// </summary>
    public struct CharacterStatusBarInfo
    {
        public int hp;
        public int hp_max;
        public int sp;
        public int sp_max;
    }

    public class EjiTosBot
    {
        /// <summary>
        /// 获取进程列表
        /// </summary>
        [DllImport("Horus.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void GetProcessList(
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_INT)]
            out int[] pid_arr,
            [Out, MarshalAs(UnmanagedType.SafeArray, SafeArraySubType = VarEnum.VT_BSTR)]
            out string[] proc_name_arr);

        /// <summary>
        /// 游戏初始化
        /// </summary>
        [DllImport("Horus.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void GameCaptureInit(int game_pid, ref CharacterStatusBarLoc csb_loc);

        /// <summary>
        /// 获取游戏角色状态信息
        /// </summary>
        [DllImport("Horus.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void GetStatusBarInfoFromOCR(int game_pid, CharacterStatusBarLoc csb_loc, out CharacterStatusBarInfo csb_info);

        /// <summary>
        /// 获取游戏角色状态信息
        /// </summary>
        [DllImport("Horus.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void GetStatusBarInfoFromLine(int game_pid, CharacterStatusBarLoc csb_loc, out CharacterStatusBarInfo csb_info);
    }
}
