using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Timers;
namespace FirstTest
{
    public static class MyTimer
    {
        static Timer _timer;
        public static bool elapsed;
        //static List<DateTime> _l;
        public static bool isElapsed
        {
            get
            {
                return elapsed;
            }
        }
        
        public static void start()
        {
            //_l = new List<DateTime>();
            
            _timer = new Timer(2000);
            
            elapsed = false;

            _timer.Elapsed += new ElapsedEventHandler(_timer_Elapsed);
            _timer.Enabled = true;
        }

        static void _timer_Elapsed(Object sender, ElapsedEventArgs e)
        {
            elapsed = true;
            _timer.Enabled = false;
           // _l.Add(DateTime.Now);
            //for (int i = 0; i < 10; i++) ;
            //elapsed = false;
        }
    }
}
