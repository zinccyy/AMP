using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AMPClient_UWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private readonly (string Tag, Type Page)[] mPages =
        {
            ("home", typeof(HomePage)),
            ("albums", typeof(AlbumsPage)),
            ("artists", typeof(ArtistsPage)),
            ("tracks", typeof(TracksPage)),
            ("folders", typeof(FoldersPage)),
        };

        private void NavView_ItemInvoked(NavigationView sender, NavigationViewItemInvokedEventArgs args)
        {
            if (args.IsSettingsInvoked == true)
            {
                NavView_Navigate("settings", args.RecommendedNavigationTransitionInfo);
            }
            else if (args.InvokedItemContainer != null)
            {
                var navItemTag = args.InvokedItemContainer.Tag.ToString();
                NavView_Navigate(navItemTag, args.RecommendedNavigationTransitionInfo);
            }
        }

        private void NavView_Navigate(string navItemTag, Microsoft.UI.Xaml.Media.Animation.NavigationTransitionInfo transitionInfo)
        {
            Type page = null;
            
            page = mPages.First(p => p.Tag.Equals(navItemTag)).Page;
            
            // Get the page type before navigation so you can prevent duplicate
            // entries in the backstack.
            var preNavPageType = ContentFrame.CurrentSourcePageType;

            // Only navigate if the selected page isn't currently loaded.
            if (!(page is null) && !Type.Equals(preNavPageType, page))
            {
                ContentFrame.Navigate(page, null, transitionInfo);
            }
        }

        private void NavView_Loaded(object sender, RoutedEventArgs e)
        {
            NavView.SelectedItem = NavView.MenuItems[0];
            NavView_Navigate("home", new Microsoft.UI.Xaml.Media.Animation.EntranceNavigationTransitionInfo());
        }

        private void NavView_BackRequested(NavigationView sender, NavigationViewBackRequestedEventArgs args)
        {

        }
    }
}
