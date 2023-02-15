using System;
using System.Windows.Forms;

namespace MemoryVisualizer
{
static class EntryProgram
{
    /// <summary>
    ///  The main entry point for the application.
    /// </summary>
    static void Main()
    {
        // To customize application configuration such as set high DPI settings or default font,
        // see https://aka.ms/applicationconfiguration.
        //ApplicationConfiguration.Initialize();
        Application.Run(new Form1());
    }    
}
}