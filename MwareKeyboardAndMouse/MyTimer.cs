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
        static bool elapsed;
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
            elapsed = false;
            _timer = new Timer(1000);
            
            _timer.Elapsed += new ElapsedEventHandler(_timer_Elapsed);
            _timer.Enabled = true;
        }

        static void _timer_Elapsed(Object sender, ElapsedEventArgs e)
        {
            _timer.Enabled = false;
            elapsed = true;
           // _l.Add(DateTime.Now);
        }
    }
}
