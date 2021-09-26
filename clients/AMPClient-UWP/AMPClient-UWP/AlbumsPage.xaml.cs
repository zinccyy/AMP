using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AMPClient_UWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class AlbumsPage : Page
    {
        public ObservableCollection<Database.Album> Albums = new ObservableCollection<Database.Album>();

        public AlbumsPage()
        {
            this.InitializeComponent();
            var app = Application.Current as App;
            var albums = app.DatabaseClient.GetAlbums(new Common.Empty());

            foreach(var album in albums.Albums)
            {
                album.Folder += "\\cover.jpg";
                Albums.Add(album);
            }
        }
    }
}
