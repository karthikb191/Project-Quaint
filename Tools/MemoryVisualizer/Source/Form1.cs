using System;
using System.Windows.Forms;
using System.IO.MemoryMappedFiles;

namespace MemoryVisualizer
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            MouseDownLocation = new System.Drawing.Point(0, 0);
            InitializeComponent();

            Refresh();
            memDisplayPanel.MouseWheel += new MouseEventHandler(memDisplayPanel_MouseWheel);
            memDisplayPanel.MouseDown += new MouseEventHandler(memDisplayPanel_MouseDown);
            memDisplayPanel.MouseUp += new MouseEventHandler(memDisplayPanel_MouseUp);
            memDisplayPanel.MouseLeave += new System.EventHandler(memDisplayPanel_MouseLeave);
            memDisplayPanel.MouseMove += new MouseEventHandler(memDisplayPanel_MouseMove);
        }

        private void memDisplayPanel_Paint(object sender, PaintEventArgs e)
        {
            Paint();
        }
        private void memDisplayPanel_MouseDown(object sender, MouseEventArgs e)
        {
            MouseDownLocation = e.Location;
            MouseDown = true;
        }
        private void memDisplayPanel_MouseUp(object sender, MouseEventArgs e)
        {
            MouseDown = false;
        }
        private void memDisplayPanel_MouseLeave(object senser, System.EventArgs args)
        {
            MouseDown = false;
        }
        private void memDisplayPanel_MouseMove(object sender, MouseEventArgs e)
        {

            float Zoom = ZoomLevel / (float)DefaultZoomLevel;
            Zoom = Math.Clamp(Zoom, 0.5f, 1000.0f);


            if (MouseDown)
            {
                OffsetX += ((float)(e.Location.X - MouseDownLocation.X)) / Zoom;
                MouseDownLocation.X = e.Location.X;
                debugLabel.Text = OffsetX.ToString();

                Refresh();
            }
            else
            {
                //Transform Pivot to the unscaled(Default) position
                float newPivot = e.X;
                float OldPivot = ZoomPivotX;
                newPivot -= ZoomPivotX;
                //newPivot = newPivot / (float)Zoom;
                newPivot += ZoomPivotX;
                ZoomPivotX = newPivot;

                debugLabel.Text = ZoomPivotX.ToString();
                ZoomPivotX = System.Math.Clamp(ZoomPivotX, 0, memDisplayPanel.Width);
                
                ZoomPivotY = e.Y;
            }
        }
        private void memDisplayPanel_MouseWheel(object sender, MouseEventArgs e)
        {
            ZoomLevel += Math.Sign(e.Delta) * ZoomSpeed;
            Refresh();
        }

        private void ReadFromSharedMemory()
        {
            MemoryMappedViewStream? stream = SharedMemoryLooker.ReadFromSharedMemoryLocation();
            if(stream == null)
            {
                return;
            }
            stream.Seek(0, System.IO.SeekOrigin.Begin);

            float Zoom = ZoomLevel / (float)DefaultZoomLevel;
            Zoom = Math.Clamp(Zoom, 0.5f, 100000.0f);

            int uint_size = UIntPtr.Size;
            byte[] buffer = new byte[uint_size * 5];
            stream.Read(buffer, 0, uint_size * 5);
            UInt64 totalSize = BitConverter.ToUInt64(buffer, 0);
            UInt64 techniqueHeaderSize = BitConverter.ToUInt64(buffer, 1 * uint_size);
            UInt64 numBlocks = BitConverter.ToUInt64(buffer, 2 * uint_size);
            UInt64 trackerBlockSize = BitConverter.ToUInt64(buffer, 3 * uint_size);
            UInt64 initialAddress = BitConverter.ToUInt64(buffer, 4 * uint_size);
            UInt64 finalAddress = initialAddress + totalSize;

            stream.Seek(uint_size * 5, System.IO.SeekOrigin.Begin);
            for(UInt64 i = 0; i < numBlocks; i++)
            {
                buffer = new byte[trackerBlockSize];
                stream.Read(buffer, 0, (int)trackerBlockSize);
                //stream.Seek((long)trackerBlockSize, System.IO.SeekOrigin.Current);

                string text = System.Text.Encoding.Default.GetString(buffer, 0, 64);
                UInt64 startAddress = BitConverter.ToUInt64(buffer, 64);
                UInt64 blockSize = BitConverter.ToUInt64(buffer, 64 + uint_size);
                bool used = BitConverter.ToBoolean(buffer, 64 + uint_size + uint_size);

                //Header
                System.Drawing.Graphics graphics = memDisplayPanel.CreateGraphics();
                float width = ((float)techniqueHeaderSize / (float)totalSize) * memDisplayPanel.Width;
                width *= Zoom;
                float height = memDisplayPanel.Height;

                float x = ((float)(finalAddress - startAddress) / (float)totalSize);
                x = (1.0f - x) * memDisplayPanel.Width;
                float y = 0;

                x -= memDisplayPanel.Width / 2.0f - OffsetX;
                x = x * Zoom;
                x += memDisplayPanel.Width / 2.0f;

                System.Drawing.RectangleF rect =
                    new System.Drawing.RectangleF(x, y, width, height);
                System.Drawing.SolidBrush brush =
                    new System.Drawing.SolidBrush(System.Drawing.Color.Violet);
                graphics.FillRectangle(brush, rect);

                //Data
                width = ((float)blockSize / (float)totalSize) * memDisplayPanel.Width;
                width *= Zoom;
                height = memDisplayPanel.Height;

                x = ((float)(finalAddress - (startAddress + techniqueHeaderSize)) / (float)totalSize);
                x = (1.0f - x) * memDisplayPanel.Width;
                y = 0;

                x -= memDisplayPanel.Width / 2.0f - OffsetX;
                x = x * Zoom;
                x += memDisplayPanel.Width / 2.0f;

                rect =
                    new System.Drawing.RectangleF(x, y, width, height);
                System.Drawing.Color color = used ? System.Drawing.Color.Red : System.Drawing.Color.Green;
                brush =
                    new System.Drawing.SolidBrush(color);
                graphics.FillRectangle(brush, rect);

            }

        }

        private void Paint()
        {
            //Populate memory display panel with contents
            int divisions = 10;
            int panelWidth = memDisplayPanel.Width;
            int panelHeight = memDisplayPanel.Height;
            System.Drawing.Graphics graphics = memDisplayPanel.CreateGraphics();

            ReadFromSharedMemory();
            //For Debug purposes
            //System.Drawing.Color[] Colors =
            //{
            //    System.Drawing.Color.Red,
            //    System.Drawing.Color.Blue,
            //    System.Drawing.Color.Green,
            //    System.Drawing.Color.Violet,
            //    System.Drawing.Color.Yellow,
            //    System.Drawing.Color.YellowGreen,
            //};
            //
            //float Zoom = ZoomLevel / (float)DefaultZoomLevel;
            //Zoom = Math.Clamp(Zoom, 0.5f, 1000.0f);
            //
            //for (int i = 0; i < divisions; i++)
            //{
            //    //x and y are relative to panel
            //    float x = (i * (panelWidth / (float)divisions)); //Original
            //
            //    x -= memDisplayPanel.Width/2.0f - OffsetX;
            //    x = x * Zoom;
            //    x += memDisplayPanel.Width/2.0f;
            //
            //    float y = 0;
            //    float width = (float)(panelWidth / 10) * Zoom;
            //    float height = panelHeight;
            //    System.Drawing.RectangleF rect =
            //        new System.Drawing.RectangleF(x, y, width, height);
            //    //Grap
            //    System.Drawing.SolidBrush brush =
            //        new System.Drawing.SolidBrush(Colors[ i % Colors.Length ]);
            //    graphics.FillRectangle(brush, rect);
            //}
        }
        private System.Drawing.Point MouseDownLocation;
        private bool MouseDown = false;
        private float OffsetX = 0;
        
        private float ZoomPivotX = 0;
        private float ZoomPivotY = 0;
        private float DefaultZoomLevel = 50;
        private float ZoomLevel = 50;
        private float ZoomSpeed = 500;
    }
}