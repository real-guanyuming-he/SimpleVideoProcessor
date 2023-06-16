using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SimpleVideoProcessorCSharp
{
    /// <summary>
    /// Represents a clip piece starting from Start and ends at End
    /// Just a container. Does not provide any check.
    /// 
    /// Rep Invariance: 
    ///     Start and End are in hh:mm:ss.fff format
    ///     and Start is less than End
    /// </summary>
    class ClipPiece
    {
        public ClipPiece() :
            this(0.0, 0.1)
        { 
        }

        /// <param name="start">in hh:mm:ss.fff format</param>
        /// <param name="end">in hh:mm:ss.fff format</param>
        /// <exception cref="ArgumentException">if start is bigger than or equal to end</exception>
        public ClipPiece(String start, String end)
        {
            if (StringHmsToTimeSpan(start) >= StringHmsToTimeSpan(end))
            {
                throw new ArgumentException("End must > Start.");
            }

            Start = start;
            End = end;
        }

        /// <param name="start">time in seconds</param>
        /// <param name="end">time in seconds</param>
        /// <exception cref="ArgumentException">if start is bigger than or equal to end</exception>
        public ClipPiece(double start, double end)
        {
            if(start >= end)
            {
                throw new ArgumentException("End must > Start.");
            }

            Start = SecondsToHms(start);
            End = SecondsToHms(end);
        }

        public ClipPiece(TimeSpan start, TimeSpan end)
        {
            if (start >= end)
            {
                throw new ArgumentException("End must > Start.");
            }

            Start = StrFromTimeSpan(start);
            End = StrFromTimeSpan(end);
        }

        public String Start;
        public String End;

        public void SetStartFromTimeSpan(TimeSpan start) 
        {
            Start = StrFromTimeSpan(start);
        }

        public void SetEndFromTimeSpan(TimeSpan end)
        {
            End = StrFromTimeSpan(end);
        }

        /// <returns>Start in seconds</returns>
        public double GetStartSec()
        {
            return GetStartTimeSpan().TotalSeconds;
        }

        /// <returns>End in seconds</returns>
        public double GetEndSec()
        {
            return GetEndTimeSpan().TotalSeconds;
        }

        public TimeSpan GetStartTimeSpan()
        {
            return StringHmsToTimeSpan(Start);
        }
        public TimeSpan GetEndTimeSpan()
        {
            return StringHmsToTimeSpan(End);
        }

        /// <summary>
        /// Parses time into TimeSpan
        /// </summary>
        /// <param name="time">in hh:mm:ss.fff</param>
        /// <returns>timespan parsed from time</returns>
        static private TimeSpan StringHmsToTimeSpan(String time)
        {
            return TimeSpan.Parse(time, new CultureInfo("en-US"));
        }

        /// <summary>
        /// Formats time in seconds to hh:mm:ss.fff
        /// </summary>
        /// <param name="seconds">time to format</param>
        /// <returns>a formatted string</returns>
        static String SecondsToHms(double seconds)
        {
            TimeSpan time = TimeSpan.FromSeconds(seconds);
            return StrFromTimeSpan(time);
        }

        /// <summary>
        /// Converts a time span to a string in the required format
        /// </summary>
        /// <param name="time"></param>
        /// <returns>a formatted string</returns>
        static String StrFromTimeSpan(TimeSpan time)
        {
            return time.ToString(@"hh\:mm\:ss\.fff");
        }
    }
}
