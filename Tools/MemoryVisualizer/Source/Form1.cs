using System.Windows.Forms;
using System;

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
                OffsetX += ((float)(e.Location.X - MouseDownLocation.X));
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

            float Zoom = ZoomLevel / (float)DefaultZoomLevel;
            Zoom = Math.Clamp(Zoom, 0.5f, 1000.0f);
            
            for (int i = 0; i < divisions; i++)
            {
                //x and y are relative to panel
                float x = (i * (panelWidth / (float)divisions)); //Original

                x -= memDisplayPanel.Width/2.0f - OffsetX;
                x = x * Zoom;
                x += memDisplayPanel.Width/2.0f;

                float y = 0;
                float width = (float)(panelWidth / 10) * Zoom;
                float height = panelHeight;
                System.Drawing.RectangleF rect =
                    new System.Drawing.RectangleF(x, y, width, height);
                //Grap
                System.Drawing.SolidBrush brush =
                    new System.Drawing.SolidBrush(Colors[ i % Colors.Length ]);
                graphics.FillRectangle(brush, rect);
            }
        }
        private System.Drawing.Point MouseDownLocation;
        private bool MouseDown = false;
        private float OffsetX = 0;
        
        private float ZoomPivotX = 0;
        private float ZoomPivotY = 0;
        private float DefaultZoomLevel = 50;
        private float ZoomLevel = 50;
        private float ZoomSpeed = 3;
    }
}