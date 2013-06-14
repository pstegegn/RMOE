using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace FirstTest
{
    class KEYBDControl
    {
        public const int KeyUp = 0x0002;
        public const int KeyDown = 0x0000;
        public const int ExtendedKey = 0x0001;
        public const int KEYEVENTF_SCANCODE = 0x0008;


        public enum KEYBDkeys
        {
            tab = 0x09, enter = 0x0D, shift = 0x10,
            ctrl = 0x11, alt = 0x12, capslock = 0x14,
            esc = 0x1B, space = 0x20, left = 0x25,
            up = 0x26, right = 0x27, down = 0x28
        };

        internal struct KEYBDINPUT
        {
            public ushort wVk;
            public ushort wScan;
            public uint dwFlags;
            public long time;
            public uint dwExtraInfo;
        }

        [StructLayout(LayoutKind.Explicit, Size = 28)]
        internal struct INPUT
        {
            [FieldOffset(0)]
            public uint type;
            [FieldOffset(4)]
            public KEYBDINPUT ki;
        }

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

        //[DllImport("user32.dll", EntryPoint = "GetMessageExtraInfo", SetLastError = true)]
        //static extern IntPtr GetMessageExtraInfo();

        public static void keyDown(ushort key)
        {
            INPUT[] InputList = new INPUT[1];

            InputList[0] = new INPUT();
            InputList[0].type = 1;

            //Keys virtualKeycode = (Keys)0x15;

            InputList[0].ki.wVk = key;
            InputList[0].ki.wScan = 0;
            InputList[0].ki.time = 0;
            InputList[0].ki.dwFlags = 0;
            InputList[0].ki.dwExtraInfo = (uint)IntPtr.Zero;

            uint stat = SendInput((uint)1, InputList, Marshal.SizeOf(InputList[0]));
            Console.WriteLine("\nresult {0} , ", stat);
        }
        public static void keyUp(ushort key)
        {
            INPUT[] InputList = new INPUT[1];

            InputList[0] = new INPUT();
            InputList[0].type = 1;

            //Keys virtualKeycode = (Keys)0x15;

            InputList[0].ki.wVk = key;
            InputList[0].ki.wScan = 0;
            InputList[0].ki.time = 0;
            InputList[0].ki.dwFlags = 2;
            InputList[0].ki.dwExtraInfo = (uint)IntPtr.Zero;

            // keyInput.ki.dwFlags = (int)KeyEvent.KeyUp;

            // InputList[1] = keyInput;

            uint stat = SendInput((uint)1, InputList, Marshal.SizeOf(InputList[0]));
            Console.WriteLine("\nresult {0} , ", stat);
        }
        public static void tskMgr()
        {
            KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.ctrl); //ctrl
            KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.shift); //alt
            KEYBDControl.pressKey((ushort)KEYBDControl.KEYBDkeys.esc); //del

            KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.ctrl); //ctrl
            KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.shift); //alt

        }
        public static void pressKey(ushort key)
        {

            INPUT[] InputList = new INPUT[2];

            InputList[0] = new INPUT();
            InputList[0].type = 1;

            //Keys virtualKeycode = (Keys)0x15;

            InputList[0].ki.wVk = key;
            InputList[0].ki.wScan = 0;
            InputList[0].ki.time = 0;
            InputList[0].ki.dwFlags = 0;
            InputList[0].ki.dwExtraInfo = (uint)IntPtr.Zero;


            InputList[1] = new INPUT();
            InputList[1].type = 1;

            //Keys virtualKeycode = (Keys)0x15;

            InputList[1].ki.wVk = key;
            InputList[1].ki.wScan = 0;
            InputList[1].ki.time = 0;
            InputList[1].ki.dwFlags = 2;
            InputList[1].ki.dwExtraInfo = (uint)IntPtr.Zero;

            // keyInput.ki.dwFlags = (int)KeyEvent.KeyUp;

            // InputList[1] = keyInput;

            uint stat = SendInput((uint)2, InputList, Marshal.SizeOf(InputList[0]));
            Console.WriteLine("\nresult {0} , ", stat);
        }
    

    }
}
