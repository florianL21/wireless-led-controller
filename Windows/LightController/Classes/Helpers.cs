using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Net.Http;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using Newtonsoft.Json;

namespace LightController
{
    class httpHelper
    {
        private static readonly HttpClient client = new HttpClient();

        private string IpAddress { get; set; }

        public httpHelper(string IP)
        {
            IpAddress = IP;
        }

        async public Task<bool> sendObject(object objectToSend, string sendPath)
        {
            bool success = false;
            try
            {
                var response = await client.PostAsync("http://" + IpAddress + "/" + sendPath, new StringContent(JsonConvert.SerializeObject(objectToSend), Encoding.UTF8, "application/json"));
                if (response.StatusCode != System.Net.HttpStatusCode.OK)
                {
                    var responseString = await response.Content.ReadAsStringAsync();
                    MessageBox.Show(responseString);
                }
                else
                {
                    success = true;
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show("Error while sending:\n" + ex.Message);
            }
            return success;
        }

        async public Task<object> getObject(object objectToSend, string getPath)
        {
            object returnObject = null;
            try
            {
                var response = await client.GetAsync("http://" + IpAddress + "/" + getPath);
                if (response.StatusCode != System.Net.HttpStatusCode.OK)
                {
                    var responseString = await response.Content.ReadAsStringAsync();
                    MessageBox.Show(responseString);
                }
                else
                {

                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error while sending:\n" + ex.Message);
            }
            return returnObject;
        }
    }

    public class colorConverter
    {
        static public int ConvertToInt(SolidColorBrush color)
        {
            string hexColor = string.Format("{0:X2}{1:X2}{2:X2}", color.Color.R, color.Color.G, color.Color.B);
            return int.Parse(hexColor, System.Globalization.NumberStyles.HexNumber);
        }

        static public SolidColorBrush ConvertToColor(int color)
        {
            string hexColor = '#' + color.ToString("X6");
            Color convertedColor = (Color)ColorConverter.ConvertFromString(hexColor);
            return new SolidColorBrush(convertedColor);
        }
    }

    public class LedStripe
    {
        public string Name
        {
            get;
            set;
        }

        public ObservableCollection<LedItem> LedItems
        {
            get;
            set;
        }
    }

    public class LedItem : INotifyPropertyChanged
    {
        private bool _selected;
        private SolidColorBrush _Color;

        public event PropertyChangedEventHandler PropertyChanged;

        public int Id
        {
            get;
            set;
        }

        public SolidColorBrush Color
        {
            get
            {
                return _Color;
            }
            set
            {
                _Color = value;
                NotifyPropertyChanged("Color");
            }
        }

        public SolidColorBrush ForeColor
        {
            get
            {
                if (Color == Brushes.Black)
                {
                    return Brushes.SteelBlue;
                }
                else
                {
                    return Brushes.White;
                }
            }
        }

        public bool selected
        {
            get
            {
                return _selected;
            }
            set
            {
                _selected = value;
                NotifyPropertyChanged("selected");
                NotifyPropertyChanged("BorderColor");
            }
        }

        public SolidColorBrush BorderColor
        {
            get
            {
                if (_selected)
                {
                    return Brushes.SteelBlue;
                }
                else
                {
                    return Brushes.White;
                }
            }
        }

        public LedItem()
        {
            selected = false;
        }

        private void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

}
