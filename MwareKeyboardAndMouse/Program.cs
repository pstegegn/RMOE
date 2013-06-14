#pragma warning disable 0649

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Text;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Threading;
using System.Timers;

namespace FirstTest
{
    internal struct MouseInput
    {
        public int X;
        public int Y;
        public int MouseData;
        public uint Flags;
        public uint Time;
        public IntPtr ExtraInfo;
    }

    internal struct Input
    {
        public int Type;
        public MouseInput MouseInput;
    }

    public static class NativeMethods
    {
        public const int InputMouse = 0;

        public const int MouseEventMove = 0x01;
        public const int MouseEventLeftDown = 0x02;
        public const int MouseEventLeftUp = 0x04;
        public const int MouseEventRightDown = 0x08;
        public const int MouseEventRightUp = 0x0010;
        public const int MOUSEEVENTF_MIDDLEDOWN = 0x0020;
        public const int MOUSEEVENTF_MIDDLEUP = 0x0040;
        public const int MOUSEEVENTF_XDOWN = 0x0080;
        public const int MOUSEEVENTF_XUP = 0x0100;
        public const int MOUSEEVENTF_WHEEL = 0x0800;
        public const int MOUSEEVENTF_HWHEEL = 0x1000;
        public const int MOUSEEVENTF_MOVE_NOCOALESCE = 0x2000;
        public const int MOUSEEVENTF_VIRTUALDESK = 0x4000;
        public const int MouseEventAbsolute = 0x8000;

        public const int XBUTTON1 = 0x0001;
        public const int XBUTTON2 = 0x0002;

        public const int WHEEL_DELTA = 120;

        public enum MouseButton : int { RIGHT, LEFT, MIDDLE, X1, X2 };
        public enum WheelPosition : int { VERTICAL, HORIZONTAL };

        private static bool lastLeftDown;

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint SendInput(uint numInputs, Input[] inputs, int size);

        [DllImport("user32.dll")]
        private static extern bool SetCursorPos(int X, int Y);

        public static void SendMouseInput(int positionX, int positionY, int maxX, int maxY, bool leftDown)
        {
            if (positionX > int.MaxValue)
                throw new ArgumentOutOfRangeException("positionX");
            if (positionY > int.MaxValue)
                throw new ArgumentOutOfRangeException("positionY");

            Input[] i = new Input[2];

            // move the mouse to the position specified
            i[0] = new Input();
            i[0].Type = InputMouse;
            i[0].MouseInput.X = (positionX * 65535) / maxX;
            i[0].MouseInput.Y = (positionY * 65535) / maxY;
            i[0].MouseInput.Flags = MouseEventAbsolute | MouseEventMove;

            // determine if we need to send a mouse down or mouse up event
            if (!lastLeftDown && leftDown)
            {
                i[1] = new Input();
                i[1].Type = InputMouse;
                i[1].MouseInput.Flags = MouseEventLeftDown;
            }
            else if (lastLeftDown && !leftDown)
            {
                i[1] = new Input();
                i[1].Type = InputMouse;
                i[1].MouseInput.Flags = MouseEventLeftUp;
            }

            lastLeftDown = leftDown;

            // send it off
            uint result = SendInput(2, i, Marshal.SizeOf(i[0]));
            if (result == 0)
                throw new Win32Exception(Marshal.GetLastWin32Error());
        }

        public static void moveCursor(int x, int y)
        {
            SetCursorPos(x, y);
        }
        public static void press(MouseButton selected)
        {

            Input i = new Input();

            i.Type = InputMouse;
            if (selected == MouseButton.LEFT)
            {
                i.MouseInput.Flags = MouseEventLeftDown;
            }
            else if (selected == MouseButton.RIGHT)
            {
                i.MouseInput.Flags = MouseEventRightDown;
            }
            else if (selected == MouseButton.MIDDLE)
            {
                i.MouseInput.Flags = MOUSEEVENTF_MIDDLEDOWN;
            }
            else if (selected == MouseButton.X1)
            {
                i.MouseInput.MouseData = XBUTTON1;
                i.MouseInput.Flags = MOUSEEVENTF_XDOWN;
            }
            else if (selected == MouseButton.X2)
            {
                i.MouseInput.MouseData = XBUTTON2;
                i.MouseInput.Flags = MOUSEEVENTF_XDOWN;
            }



            Input[] ia = new Input[1];
            ia[0] = i;
            // send it off
            uint result = SendInput(1, ia, Marshal.SizeOf(i));
            if (result == 0)
                throw new Win32Exception(Marshal.GetLastWin32Error());

        }

        public static void release(MouseButton selected)
        {

            Input i = new Input();

            i.Type = InputMouse;
            if (selected == MouseButton.LEFT)
            {
                i.MouseInput.Flags = MouseEventLeftUp;
            }
            else if (selected == MouseButton.RIGHT)
            {
                i.MouseInput.Flags = MouseEventRightUp;
            }
            else if (selected == MouseButton.MIDDLE)
            {
                i.MouseInput.Flags = MOUSEEVENTF_MIDDLEUP;
            }
            else if (selected == MouseButton.X1)
            {
                i.MouseInput.MouseData = XBUTTON1;
                i.MouseInput.Flags = MOUSEEVENTF_XUP;
            }
            else if (selected == MouseButton.X2)
            {
                i.MouseInput.MouseData = XBUTTON2;
                i.MouseInput.Flags = MOUSEEVENTF_XUP;
            }

            Input[] ia = new Input[1];
            ia[0] = i;
            // send it off
            uint result = SendInput(1, ia, Marshal.SizeOf(i));
            if (result == 0)
                throw new Win32Exception(Marshal.GetLastWin32Error());

        }

        public static void singleClick(MouseButton selected)
        {
            press(selected);
            release(selected);
        }

        public static void doubleClick(MouseButton selected)
        {
            singleClick(selected);
            singleClick(selected);
        }

        public static void scroll(WheelPosition pos, int amount)
        {
            Input i = new Input();

            i.Type = InputMouse;
            if (pos == WheelPosition.HORIZONTAL)
            {
                i.MouseInput.Flags = MOUSEEVENTF_HWHEEL;
            }
            else if (pos == WheelPosition.VERTICAL)
            {
                i.MouseInput.Flags = MOUSEEVENTF_WHEEL;
            }

            i.MouseInput.MouseData = amount * WHEEL_DELTA - 1;
            Input[] ia = new Input[1];
            ia[0] = i;
            // send it off
            uint result = SendInput(1, ia, Marshal.SizeOf(i));
            if (result == 0)
                throw new Win32Exception(Marshal.GetLastWin32Error());
        }
    }
 
    //middleware wrapper structures
    [StructLayout(LayoutKind.Sequential)]
    public struct Point3D
    {
        public float x;
        public float y;
        public float z;
        public float confidence;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Point2D
    {
       public float x;
       public  float y;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct SkeletonPoint
    {
        public int id;
        public Point3D Head;
        public Point3D RightHand;
        public Point3D LeftHand;
        public Point3D Torso;
        public int RightHandFingers;
        public int LeftHandFingers;
        public int RightHandGusture;
    };

    class Program
    {
        //callback definition from from c++ DLL..........................//
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ProgressCallback(int value);

        [DllImport(@"C:\\Users\\paulos\\Documents\\Visual Studio 2010\\Projects\\RMOE_Thesis_Resources\\MwareInterface\\Debug\\MwareInterface.dll")]
        public static extern int Init();

        [DllImport(@"C:\\Users\\paulos\\Documents\\Visual Studio 2010\\Projects\\RMOE_Thesis_Resources\\MwareInterface\\Debug\\MwareInterface.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Update(ref SkeletonPoint skeleton);

        [DllImport(@"C:\\Users\\paulos\\Documents\\Visual Studio 2010\\Projects\\RMOE_Thesis_Resources\\MwareInterface\\Debug\\MwareInterface.dll")]
        public static extern void Shutdown();

       
        //-----------------------------------------------------------------------------//
        public static bool Terminate = false;
        public static SkeletonPoint skeleton = new SkeletonPoint();
        public static int prevGesture = 0;
        public static float x = 0;
        public static float y = 0;

        public static bool isAltPressed = false;
        public static bool isctrlPressed = false;

        public static bool isGesture = true;
        public static bool isMiddlePressed = false;
        
        //mode States      
        public static int State = 0;

        public static Point2D[] points = new Point2D[10];
        public static Point2D pointsAvg = new Point2D();
        [STAThread]
        static void Main(string[] args)
        {
            //-----------------------starting------------------------------------------//
            //KEYBDControl.pressKey((ushort)KEYBDControl.KEYBDkeys.alt);

            //MyTimer.start();
            //while(true){
            //    System.Console.WriteLine(MyTimer.isElapsed);
            //}

            int status = Init();
            System.Console.WriteLine(" => {0}", status);

            {
                int maxx = (int)SystemInformation.PrimaryMonitorSize.Width;
                int maxy = (int)SystemInformation.PrimaryMonitorSize.Height;
                //SystemInformation.
                Console.WriteLine("Monitor size ===> width = {0} height = {1} ", maxx, maxy);
            }

            if (status != 0)
            {
                Shutdown();
                System.Console.WriteLine("Plug the Kinect properly and restart this program!");
                System.Console.WriteLine("press Enter to Exit!");
                System.Console.Read();
                return;
            }
            //-------------------------------------------------------------------------//
            
            Thread MainThread = new Thread(new ThreadStart(startProcess));
            Thread ConsoleKeyListener = new Thread(new ThreadStart(ListerKeyBoardEvent));
            Thread.CurrentThread.Name = "Current";
            MainThread.Name = "Processor";
            ConsoleKeyListener.Name = "KeyListener";
            MainThread.Start();
            ConsoleKeyListener.Start();

            //------Timer------------------------------------------------------------//
            
          
            while (true)
            {
                if (Terminate)
                {
                    Console.WriteLine("Terminating Process...");
                    MainThread.Abort();
                    ConsoleKeyListener.Abort();
                    Thread.Sleep(1000);
                    Thread.CurrentThread.Abort();
                    break;
                }
            } //end of while loop
            Shutdown();
            System.Console.WriteLine("end");
            return;
        }

        //keypress listener
        public static void ListerKeyBoardEvent()
        {
            do
            {
                if (Console.ReadKey(true).Key == ConsoleKey.Escape)
                {
                    Terminate = true;
                }
            } while (true);
        }       
        //work
        public static void startProcess()
        {
            while (true)
            {
                if (!Terminate)
                {
                    //process
                    Update(ref skeleton);
                   // KEYBDControl
                    Command();
                    Thread.Sleep(0);
                }
            }
        }

        public static void Command()
        {
            ShiftAndAverageXY();
            //set cursor position
            if(isAltPressed && isctrlPressed)
            {
                NativeMethods.moveCursor((int)x, -(int)y);
            }
            else
            {
                NativeMethods.moveCursor((int)x * 4, -(int)y * 4);
            }

            if (MyTimer.isElapsed)
            {
                isGesture = true;
            }

            //Track the state based on the gesture
            TrackGestureState();
             
            switch (State) {
                //General mouse control Mode
                case 0:
                    ModeGeneralMouseInterface();
                    break;
                //General mouse control Mode
                case 1:
                    Mode3DInterface();
                    break;
                //General mouse control Mode
                case 2:
                    
                    break;
            }

        }

        public static void ShiftAndAverageXY()
        {
             // shift array
            for (int i = points.Length - 1; i > 0; i--)
            {
                points[i] = points[i - 1];
            }

            points[0].x = skeleton.RightHand.x;
            points[0].y = skeleton.RightHand.y;

            for (int i = 0; i < points.Length; i++)
            {
                x += points[i].x;
                y += points[i].y;
            }

            // final point is average of points 
            // in smoothing result
            x = x / points.Length;
            y = y / points.Length;
        }

        public static void TrackGestureState()
        {

            if (skeleton.LeftHand.y < skeleton.Head.y && skeleton.RightHand.y - skeleton.Head.y > 150 && State != 0 &&
                    System.Math.Abs(skeleton.LeftHand.y - skeleton.RightHand.y) > 300 &&
                    skeleton.Head.confidence > 0 && skeleton.RightHand.confidence > 0 && skeleton.LeftHand.confidence > 0)
            {
                State = 0;
                System.Console.WriteLine("00000000000000000000000000000");
            }

            if (skeleton.LeftHand.y - skeleton.Head.y > 150 && skeleton.Head.y > skeleton.RightHand.y && State != 1 &&
                System.Math.Abs(skeleton.LeftHand.y - skeleton.RightHand.y) > 300 &&
                skeleton.Head.confidence > 0 && skeleton.RightHand.confidence > 0 && skeleton.LeftHand.confidence > 0)
            {
                State = 1;
                System.Console.WriteLine("11111111111111111111111111111");
            }

            if (skeleton.LeftHand.y - skeleton.Head.y > 150 && skeleton.RightHand.y - skeleton.Head.y > 150 && State != 2 &&
                skeleton.Head.confidence > 0 && skeleton.RightHand.confidence > 0 && skeleton.LeftHand.confidence > 0)
            {
                State = 2;
                System.Console.WriteLine("22222222222222222222222222222");

            }
        }

        //the interface function for 3d app
        public static void Mode3DInterface() {

            //left hand Gesture
            if (skeleton.RightHandGusture == 1 || skeleton.RightHandFingers == 5)
            {
                //middle button
                if (isGesture)
                {
                    MyTimer.start();
                    isGesture = false;
                    if (!isMiddlePressed)
                    {
                        NativeMethods.press(NativeMethods.MouseButton.MIDDLE);
                        System.Console.WriteLine("______________________Middle 1");
                        isMiddlePressed = true;
                    }
                    else
                    {
                        isMiddlePressed = false;
                        NativeMethods.release(NativeMethods.MouseButton.MIDDLE);
                        System.Console.WriteLine("______________________Middle 0");

                    }
                }

            }
            else if (skeleton.RightHandGusture == 10)
            {
                //Left single click
                 if (isGesture)
                {

                    MyTimer.start();
                    isGesture = false;
                    NativeMethods.singleClick(NativeMethods.MouseButton.LEFT);
                }

            }

                
            //Right Hand gesture
            if (skeleton.Torso.y < skeleton.LeftHand.y)
            {
                //System.Console.WriteLine(skeleton.Torso.z - skeleton.LeftHand.z);
                //alt pressed
                if ( (skeleton.Torso.z - skeleton.LeftHand.z) < 250)
                {
                    if (!isAltPressed)
                    {
                        KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.alt);
                        isAltPressed = true;
                        System.Console.WriteLine("__________________________ALT 1");
                    }

                    if (isctrlPressed)
                    {
                        KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.ctrl);
                        isctrlPressed = false;
                        System.Console.WriteLine("__________________________CTRL 0");
                    }
                }
                //ctrl + alt pressed
                else if ((skeleton.Torso.z - skeleton.LeftHand.z) > 300)
                {
                    if (!isctrlPressed)
                    {
                        KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.ctrl);
                        System.Console.WriteLine("__________________________CTRL 1");
                        isctrlPressed = true;
                    }
  
                    if (!isAltPressed)
                    {
                        KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.alt);
                        isAltPressed = true;
                        System.Console.WriteLine("__________________________ALT 1");
                    }
                }

            }
            else
            {
                //release
                if (isAltPressed)
                {
                    KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.alt);
                    isAltPressed = false;
                    System.Console.WriteLine("__________________________ALT 0");
                    
                }
                if (isctrlPressed)
                {
                    KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.ctrl);
                    isctrlPressed = false;
                    System.Console.WriteLine("__________________________CTRL 0");
                    
                }
            }
        }

        public static void ModeGeneralMouseInterface()
        {
            //scroll
            if (skeleton.RightHandGusture == 1 || skeleton.RightHandFingers >= 4)
            {
                if (isGesture)
                {
                    MyTimer.start();
                    isGesture = false;

                    if (!isMiddlePressed)
                    {
                        NativeMethods.press(NativeMethods.MouseButton.MIDDLE);
                        isMiddlePressed = false;
                    }
                }
            }

            //left click
            else if (skeleton.RightHandGusture == 10)
            {
                if (isGesture)
                {
                    MyTimer.start();
                    isGesture = false;
                    NativeMethods.singleClick(NativeMethods.MouseButton.LEFT);
                }
            }

            //left double click
            else if (skeleton.RightHandGusture == 100)
            {
                if (isGesture)
                {

                    MyTimer.start();
                    isGesture = false;
                    NativeMethods.doubleClick(NativeMethods.MouseButton.LEFT);
                }
            }

            //right single click
            else if (skeleton.RightHandGusture == 1000)
            {
                if (isGesture)
                {

                    MyTimer.start();
                    isGesture = false;
                    NativeMethods.singleClick(NativeMethods.MouseButton.RIGHT);
                }
            }
            //ALT + F4
            if (System.Math.Abs(points[0].x - points[1].x) > 100){
                if (isGesture)
                {

                    MyTimer.start();
                    isGesture = false;
                    KEYBDControl.keyDown((ushort)KEYBDControl.KEYBDkeys.alt);
                    KEYBDControl.pressKey((ushort)Keys.F4);
                    KEYBDControl.keyUp((ushort)KEYBDControl.KEYBDkeys.alt);
                    System.Console.WriteLine(points[0].x - points[1].x);
                }
            }

            prevGesture = skeleton.RightHandGusture;

            //update the prev value
            pointsAvg.x = x;
            pointsAvg.y = y;
        }

        public static void ModeGaming() { 
            
        }


    }
}



