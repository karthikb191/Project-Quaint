using System.Windows.Forms;
using System;

namespace MemoryVisualizer
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            rand = new System.Random(100);
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
            if(MouseDown)
            {
                float Zoom = ZoomLevel / (float)DefaultZoomLevel;
                OffsetX += (int)((float)(e.Location.X - MouseDownLocation.X));
                MouseDownLocation.X = e.Location.X;
                debugLabel.Text = OffsetX.ToString();
                Refresh();
            }
        }
        private void memDisplayPanel_MouseWheel(object sender, MouseEventArgs e)
        {
            debugLabel.Text = e.X.ToString() + "   " + e.Y.ToString();
            float Zoom = (float)ZoomLevel / (float)DefaultZoomLevel;
            
            //Transform Pivot to the unscaled(Default) position
            {
                int newPivot = e.X;
                newPivot -= ZoomPivotX;
                newPivot = (int)(newPivot / (float)Zoom);
                newPivot += ZoomPivotX;
                ZoomPivotX = newPivot;
            }
            ZoomPivotY = (int)(e.Y);
            ZoomLevel += Math.Sign(e.Delta);
            Refresh();
        }

        private void Paint()
        {
            //Populate memory display panel with contents
            int divisions = 10;
            int panelWidth = memDisplayPanel.Width;
            int panelHeight = memDisplayPanel.Height;
            System.Drawing.Graphics graphics = memDisplayPanel.CreateGraphics();
            
            System.Drawing.Color[] Colors =
            {
                System.Drawing.Color.Red,
                System.Drawing.Color.Blue,
                System.Drawing.Color.Green,
                System.Drawing.Color.Violet,
                System.Drawing.Color.Yellow,
                System.Drawing.Color.YellowGreen,
            };

            float Zoom = (float)ZoomLevel / (float)DefaultZoomLevel;
            for (int i = 0; i < divisions; i++)
            {
                //x and y are relative to panel
                int x = (int)(i * ((float)panelWidth / (float)divisions)); //Original
                x -= ZoomPivotX;
                x = (int)(x * Zoom);
                x += ZoomPivotX + OffsetX;
                int y = 0;
                int width = (int)((float)(panelWidth / 10) * Zoom);
                int height = panelHeight;
                System.Drawing.Rectangle rect =
                    new System.Drawing.Rectangle(x, y, width, height);
                //Grap
                System.Drawing.SolidBrush brush =
                    new System.Drawing.SolidBrush(Colors[ i % Colors.Length ]);
                graphics.FillRectangle(brush, rect);
            }
        }
        private System.Drawing.Point MouseDownLocation;
        private bool MouseDown = false;
        private int OffsetX = 0;
        private int ZoomPivotX = 0;
        private int ZoomPivotY = 0;
        private int DefaultZoomLevel = 50;
        private int ZoomLevel = 50;
        System.Random rand;
    }
}