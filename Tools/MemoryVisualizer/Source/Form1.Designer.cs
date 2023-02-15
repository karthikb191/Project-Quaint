namespace MemoryVisualizer
{

partial class Form1
{

    private const int ButtonPaddingX = 10;
    private const int ButtonPaddingY = 10;
    /// <summary>
    ///  Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    ///  Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
        if (disposing && (components != null))
        {
            components.Dispose();
        }
        base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    ///  Required method for Designer support - do not modify
    ///  the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
            this.ControlPanel = new System.Windows.Forms.Panel();
            this.initConenction = new System.Windows.Forms.Button();
            this.memDisplayPanel = new System.Windows.Forms.Panel();
            this.rightPanel = new System.Windows.Forms.Panel();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.debugLabel = new System.Windows.Forms.Label();
            this.ControlPanel.SuspendLayout();
            this.rightPanel.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // ControlPanel
            // 
            this.ControlPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ControlPanel.Controls.Add(this.initConenction);
            this.ControlPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.ControlPanel.Location = new System.Drawing.Point(0, 0);
            this.ControlPanel.Margin = new System.Windows.Forms.Padding(15);
            this.ControlPanel.Name = "ControlPanel";
            this.ControlPanel.Padding = new System.Windows.Forms.Padding(15);
            this.ControlPanel.Size = new System.Drawing.Size(250, 186);
            this.ControlPanel.TabIndex = 1;
            // 
            // initConenction
            // 
            this.initConenction.Dock = System.Windows.Forms.DockStyle.Top;
            this.initConenction.Location = new System.Drawing.Point(15, 15);
            this.initConenction.Name = "initConenction";
            this.initConenction.Size = new System.Drawing.Size(218, 29);
            this.initConenction.TabIndex = 0;
            this.initConenction.Text = "InitConnection";
            this.initConenction.UseVisualStyleBackColor = true;
            // 
            // memDisplayPanel
            // 
            this.memDisplayPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.memDisplayPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.memDisplayPanel.Location = new System.Drawing.Point(3, 131);
            this.memDisplayPanel.Name = "memDisplayPanel";
            this.memDisplayPanel.Size = new System.Drawing.Size(579, 144);
            this.memDisplayPanel.TabIndex = 2;
            this.memDisplayPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.memDisplayPanel_Paint);
            // 
            // rightPanel
            // 
            this.rightPanel.Controls.Add(this.ControlPanel);
            this.rightPanel.Dock = System.Windows.Forms.DockStyle.Right;
            this.rightPanel.Location = new System.Drawing.Point(673, 0);
            this.rightPanel.Name = "rightPanel";
            this.rightPanel.Size = new System.Drawing.Size(250, 485);
            this.rightPanel.TabIndex = 3;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.memDisplayPanel, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.debugLabel, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Left;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 3;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 26.59794F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 30.92784F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 42.47423F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(585, 485);
            this.tableLayoutPanel1.TabIndex = 4;
            // 
            // debugLabel
            // 
            this.debugLabel.AutoSize = true;
            this.debugLabel.Location = new System.Drawing.Point(3, 0);
            this.debugLabel.Name = "debugLabel";
            this.debugLabel.Size = new System.Drawing.Size(50, 20);
            this.debugLabel.TabIndex = 3;
            this.debugLabel.Text = "label1";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(923, 485);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.rightPanel);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ControlPanel.ResumeLayout(false);
            this.rightPanel.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

    }

        #endregion

        private System.Windows.Forms.Panel ControlPanel;
        private System.Windows.Forms.Button initConenction;
        private System.Windows.Forms.Panel memDisplayPanel;
        private System.Windows.Forms.Panel rightPanel;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Label debugLabel;

        //public System.Windows.Forms.Button InitConnection
        //{
        //    get
        //    {
        //        return initConenction;
        //    }
        //}
        //public System.Windows.Forms.Panel MemDisplayPanel
        //{
        //    get
        //    {
        //        return memDisplayPanel;
        //    }
        //}
    }
}