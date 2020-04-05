using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Net.Http;
using System.Collections.ObjectModel;

namespace LightController
{
    /// <summary>
    /// Interaktionslogik für MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public Settings config { set; get; }

        httpHelper HttpClient { set; get; }

        public ObservableCollection<LedStripe> LedStripes { get; set; }

        private bool isShiftPressed = false;
        private bool isControlPressed = false;
        private LedItem lastSelectedItem = null;

        public int numRows { get; set; }

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;


            LedStripes = new ObservableCollection<LedStripe>();
            numRows = 1;
            config = new Settings(10, 10);
            HttpClient = null;

            setupView(config.NumLeds, numRows);
            
        }

        private void setupView(int numLEDs, int numRows)
        {
            LedStripes.Clear();
            int numLEDsPerRow = numLEDs / numRows;
            
            for (int i = 0; i < numRows; i++)
            {
                var newLedItems = new ObservableCollection<LedItem>();
                for (int j = 0; j < numLEDsPerRow; j++)
                {
                    newLedItems.Add(new LedItem() { Id = j * (i + 1), Color = Brushes.Black });
                }
                LedStripes.Add(new LedStripe() { LedItems = newLedItems, Name = "Stripe" + i });
            }
            convertDataToView();
        }

        async private Task<bool> HttpHelperSend(object objectToSend, string propertyName)
        {
            if (HttpClient == null)
            {
                HttpClient = new httpHelper(TextBox_IpAddress.Text);
            }
            return await HttpClient.sendObject(objectToSend, propertyName);
        }

        async private void Button_StoreSettings_Click(object sender, RoutedEventArgs e)
        {
            Button senderButton = sender as Button;
            senderButton.IsEnabled = false;
            await HttpHelperSend(null, "store");
            senderButton.IsEnabled = true;
        }

        private void Button_Connect_Click(object sender, RoutedEventArgs e)
        {
            
            
        }

        async private Task<bool> sendData()
        {
            await HttpHelperSend(config, "settings");
            bool success = false;
            foreach (var item in config.Frames)
            {
                success = await HttpClient.sendObject(item, "leds");
                if(!success)
                {
                    MessageBox.Show("Sendig of the configuration was aborted at frame " + item.Frame);
                    break;
                }
            }
            return success;
        }

        async private void Button_sendFrames_Click(object sender, RoutedEventArgs e)
        {
            Button senderButton = sender as Button;
            senderButton.IsEnabled = false;
            convertViewToData();
            await sendData();
            senderButton.IsEnabled = true;
        }

        public List<LedItem> getLedsAsList()
        {
            List<LedItem> leds = new List<LedItem>();
            foreach (var stripe in LedStripes)
            {
                foreach (var led in stripe.LedItems)
                {
                    leds.Add(led);
                }
            }
            return leds;
        }

        void convertDataToView()
        {
            List<LedItem> leds = getLedsAsList();
            for (int i = 0; i < config.NumLeds; i++)
            {
                leds[i].Color = colorConverter.ConvertToColor(config.Frames[config.CurrentFrame].Leds[i]);
            }
        }

        void convertViewToData()
        {
            List<LedItem> leds = getLedsAsList();
            for (int i = 0; i < config.NumLeds; i++)
            {
                config.Frames[config.CurrentFrame].Leds[i] = colorConverter.ConvertToInt(leds[i].Color);
            }
        }

        private void Button_SendSettings_Click(object sender, RoutedEventArgs e)
        {
            if(HttpClient == null)
            {
                HttpClient = new httpHelper(TextBox_IpAddress.Text);
            }
            HttpClient.sendObject(config, "settings");
        }

        private void Button_PrevFrame_Click(object sender, RoutedEventArgs e)
        {
            convertViewToData();
            if (config.CurrentFrame > 0)
            {
                config.CurrentFrame--;
            }
            convertDataToView();
        }

        private void Button_NextFrame_Click(object sender, RoutedEventArgs e)
        {
            convertViewToData();
            if (config.CurrentFrame + 1 < config.NumFrames)
            {
                config.CurrentFrame++;
            }
            convertDataToView();
        }

        private void Button_CreateView_Click(object sender, RoutedEventArgs e)
        {
            setupView(config.NumLeds, numRows);
        }

        


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Button clickedButton = e.Source as Button;
            int id = int.Parse(clickedButton.Tag.ToString());
            LedItem selectedItem = null;
            SolidColorBrush color = null;


            foreach (var item in LedStripes)
            {
                var m = item.LedItems.Where(x => x.Id == id).ToList();

                if (m.Any())
                {
                    var x = m.First();

                    if (x != null)
                    {
                        selectedItem = x;
                        break;
                    }
                }
            }
            color = selectedItem.Color;
            if (selectedItem != null)
            {
                if (!isShiftPressed && !isControlPressed)
                {
                    foreach (var item in LedStripes)
                    {
                        foreach (var led in item.LedItems)
                        {
                            led.selected = false;
                        }
                    }
                    selectedItem.selected = true;
                    color = selectedItem.Color;
                }
                else if (isControlPressed && !isShiftPressed)
                {
                    selectedItem.selected = true;
                    if (color != null)
                    {
                        if (color.Color != selectedItem.Color.Color)
                        {
                            color = null;
                        }
                    }
                }
                else if (isShiftPressed)
                {
                    List<LedItem> ledList = getLedsAsList();

                    int startIndex = ledList.IndexOf(lastSelectedItem);
                    int endIndex = ledList.IndexOf(selectedItem);

                    if (startIndex != -1 && endIndex != -1)
                    {
                        if (startIndex > endIndex)
                        {
                            int temp = endIndex;
                            endIndex = startIndex;
                            startIndex = temp;
                        }

                        for (int i = startIndex; i <= endIndex; i++)
                        {
                            ledList[i].selected = true;
                            if (color != null)
                            {
                                if (color.Color != ledList[i].Color.Color)
                                {
                                    color = null;
                                }
                            }
                        }
                    }
                    else
                    {
                        selectedItem.selected = true;
                    }
                }
                lastSelectedItem = selectedItem;
            }
            if(color == null)
            {
                ColorPicker1.SelectedColor = null;
            }
            else
            {
                ColorPicker1.SelectedColor = Color.FromArgb(color.Color.A, color.Color.R, color.Color.G, color.Color.B);
            }
            
        }

        private void ColorPicker1_SelectedColorChanged(object sender, RoutedPropertyChangedEventArgs<Color?> e)
        {
            var color = ColorPicker1.SelectedColor;

            if(color != null)
            {
                foreach (var item in LedStripes)
                {
                    var m = item.LedItems.Where(x => x.selected == true).ToList();

                    m.ForEach(x => {
                        x.Color = new SolidColorBrush(color.Value);
                    });
                }
            }
            
        }

        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.LeftCtrl:
                case Key.RightCtrl:
                    isControlPressed = true;
                    break;
                case Key.LeftShift:
                case Key.RightShift:
                    isShiftPressed = true;
                    break;
            }
            base.OnKeyDown(e);
        }

        protected override void OnKeyUp(KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.LeftCtrl:
                case Key.RightCtrl:
                    isControlPressed = false;
                    break;
                case Key.LeftShift:
                case Key.RightShift:
                    isShiftPressed = false;
                    break;
            }
            base.OnKeyUp(e);
        }
    }
}
