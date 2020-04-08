using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace LightController
{
    public class Settings : INotifyPropertyChanged
    {
        private int _Brightness;
        private int _Framerate;
        private int _NumFrames;
        private int _NumLeds;
        private int _CurrentFrame;
        private SingleFrame[] _Frames;

        public int Brightness
        {
            get
            {
                return _Brightness;
            }
            set
            {
                _Brightness = value;
                NotifyPropertyChanged("Brightness");
            }
        }
        public int Framerate
        {
            get
            {
                return _Framerate;
            }
            set
            {
                _Framerate = value;
                NotifyPropertyChanged("Framerate");
            }
        }
        public int NumFrames
        {
            get
            {
                return _NumFrames;
            }
            set
            {
                _NumFrames = value;
                updateFrameBuffer();
                NotifyPropertyChanged("NumFrames");
            }
        }
        public int NumLeds
        {
            get
            {
                return _NumLeds;
            }
            set
            {
                _NumLeds = value;
                updateFrameBuffer();
                NotifyPropertyChanged("NumLeds");
            }
        }
        [JsonIgnore]
        public int CurrentFrame
        {
            get
            {
                return _CurrentFrame;
            }
            set
            {
                _CurrentFrame = value;
                NotifyPropertyChanged("CurrentFrame");
                NotifyPropertyChanged("CurrentFramePretty");
            }
        }

        public int CurrentFramePretty
        {
            get
            {
                return _CurrentFrame + 1;
            }
            set
            {
                _CurrentFrame = value - 1;
                NotifyPropertyChanged("CurrentFramePretty");
                NotifyPropertyChanged("CurrentFrame");
            }
        }

        [JsonIgnore]
        public SingleFrame[] Frames
        {
            get
            {
                return _Frames;
            }
            set
            {
                _Frames = value;
                NotifyPropertyChanged("Frames");
            }
        }

        public Settings(int numFrames = 1, int numLEDs = 10)
        {
            //populate defualt settings
            Brightness = 255;
            Framerate = 1;
            NumFrames = numFrames;
            NumLeds = numLEDs;
            CurrentFrame = 0;
            updateFrameBuffer();
        }

        public void updateFrameBuffer()
        {
            Frames = new SingleFrame[NumFrames];
            for (int i = 0; i < NumFrames; i++)
            {
                Frames[i] = new SingleFrame(i, NumLeds);
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class SingleFrame
    {
        public int Frame { get; set; }
        public int[] Leds { get; set; }

        public SingleFrame(int FrameId, int numLeds)
        {
            Frame = FrameId;
            Leds = new int[numLeds];

            for (int i = 0; i < numLeds; i++)
            {
                Leds[i] = 0;
            }
        }
    }
}
