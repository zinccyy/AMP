using Grpc.Core;
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
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.ApplicationModel;
using Windows.UI.Popups;

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

        private async void ShowError(Exception e)
        {
            var messageDialog = new MessageDialog(e.Message);

            // Add commands and set their callbacks; both buttons use the same callback function instead of inline event handlers
            messageDialog.Commands.Add(new UICommand(
                "Try again",
                new UICommandInvokedHandler(this.CommandInvokedHandler)));
            messageDialog.Commands.Add(new UICommand(
                "Close",
                new UICommandInvokedHandler(this.CommandInvokedHandler)));

            // Set the command that will be invoked by default
            messageDialog.DefaultCommandIndex = 0;

            // Set the command to be invoked when escape is pressed
            messageDialog.CancelCommandIndex = 1;

            // Show the message dialog
            await messageDialog.ShowAsync();
        }

        private void CommandInvokedHandler(IUICommand command)
        {
        }

        private async void DownloadImage(Database.Database.DatabaseClient client, Database.Album album, string fileName)
        {
            var call = client.GetAlbumCover(new Database.AlbumRequest { Artist = album.Artist, Name = album.Name, Genre = album.Genre });
            try
            {
                StorageFolder assetsFolder = await Package.Current.InstalledLocation.GetFolderAsync("Assets");
                var file = await assetsFolder.CreateFileAsync(fileName);
                var binaryWriter = new BinaryWriter(await file.OpenStreamForWriteAsync());
                while (await call.ResponseStream.MoveNext())
                {
                    binaryWriter.Write(call.ResponseStream.Current.Data.ToByteArray());
                }
            } catch(Exception e)
            {
                ShowError(e);
            }
        }

        public AlbumsPage()
        {
            this.InitializeComponent();
            var app = Application.Current as App;
            var albums = app.DatabaseClient.GetAlbums(new Common.Empty());

            foreach(var album in albums.Albums)
            {
                var fileName = String.Format("{0} - {1}.jpg", album.Artist, album.Name);
                Albums.Add(album);
                DownloadImage(app.DatabaseClient, album, fileName);
                break;
            }
        }
    }
}
