// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using muxma = Microsoft.UI.Xaml.Media.Animation;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
using Windows.UI.Core;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SimpleVideoProcessorCSharp
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public MainWindow()
        {
            this.InitializeComponent();
        }

        private void mainPageNav_SelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args)
        {
            if (args.IsSettingsSelected == true)
            {
                NavView_Navigate("Settings", args.RecommendedNavigationTransitionInfo);
            }
            else if (args.SelectedItemContainer != null)
            {
                var navItemTag = args.SelectedItemContainer.Tag.ToString();
                NavView_Navigate(navItemTag, args.RecommendedNavigationTransitionInfo);
            }
        }

        /// <summary>
        /// Navigates to the corresponding page inside the navigation view based on the tag given
        /// </summary>
        /// <param name="navItemTag"></param>
        /// <param name="transitionInfo"></param>
        private void NavView_Navigate
        (
            string navItemTag,
            muxma.NavigationTransitionInfo transitionInfo
        )
        {
            // Find the page with the Tag
            var item = _pages.FirstOrDefault(p => p.Tag.Equals(navItemTag));
            Type _page = item.Page;

            // Get the page type before navigation so you can prevent duplicate
            // entries in the backstack.
            var preNavPageType = mainPageNavContent.CurrentSourcePageType;

            // Only navigate if the selected page isn't currently loaded.
            if (!(_page is null) && !Type.Equals(preNavPageType, _page))
            {
                mainPageNavContent.Navigate(_page, null, transitionInfo);
            }
        }

        private void mainPageNav_Loaded(object sender, RoutedEventArgs e)
        {
            // Add handler for ContentFrame navigation.
            mainPageNavContent.Navigated += OnNavigated;

            // NavView doesn't load any page by default, so load home page.
            mainPageNav.SelectedItem = mainPageNav.MenuItems[0];

            //// Accessibility
            //{
            //    // Listen to the window directly so the app responds
            //    // to accelerator keys regardless of which element has focus.
            //    Window.Current.CoreWindow.Dispatcher.AcceleratorKeyActivated +=
            //        CoreDispatcher_AcceleratorKeyActivated;

            //    Window.Current.CoreWindow.PointerPressed += CoreWindow_PointerPressed;

            //    SystemNavigationManager.GetForCurrentView().BackRequested += System_BackRequested;
            //}
        }

        private void OnNavigated(object sender, NavigationEventArgs e)
        {
            mainPageNav.IsBackEnabled = mainPageNavContent.CanGoBack;

            if (mainPageNavContent.SourcePageType == typeof(SettingsPage))
            {
                // SettingsItem is not part of NavView.MenuItems, and doesn't have a Tag.
                mainPageNav.SelectedItem = (NavigationViewItem)mainPageNav.SettingsItem;
                mainPageNav.Header = "Settings";
            }
            else if (mainPageNavContent.SourcePageType != null)
            {
                // Don't need to set the selected item

                mainPageNav.Header =
                    ((NavigationViewItem)mainPageNav.SelectedItem)?.Content?.ToString();
            }
        }

        private void mainPageNavContent_NavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        #region Going back
        private void mainPageNav_BackRequested(NavigationView sender, NavigationViewBackRequestedEventArgs args)
        {
            TryGoBack();
        }

        /// <summary>
        /// Tries to go back in the navigation view
        /// </summary>
        /// <returns></returns>
        private bool TryGoBack()
        {
            if (!mainPageNavContent.CanGoBack)
                return false;

            // Don't go back if the nav pane is overlayed.
            if (mainPageNav.IsPaneOpen &&
                (mainPageNav.DisplayMode == NavigationViewDisplayMode.Compact ||
                 mainPageNav.DisplayMode == NavigationViewDisplayMode.Minimal))
                return false;

            mainPageNavContent.GoBack();
            return true;
        }
        private void System_BackRequested(object sender, BackRequestedEventArgs e)
        {
            if (!e.Handled)
            {
                e.Handled = TryGoBack();
            }
        }
        #endregion

        #region Accessibility
        private void CoreDispatcher_AcceleratorKeyActivated(CoreDispatcher sender, AcceleratorKeyEventArgs e)
        {
            // When Alt+Left are pressed navigate back
            if (e.EventType == CoreAcceleratorKeyEventType.SystemKeyDown
                && e.VirtualKey == VirtualKey.Left
                && e.KeyStatus.IsMenuKeyDown == true
                && !e.Handled)
            {
                e.Handled = TryGoBack();
            }
        }

        private void CoreWindow_PointerPressed(CoreWindow sender, PointerEventArgs e)
        {
            // Handle mouse back button.
            if (e.CurrentPoint.Properties.IsXButton1Pressed)
            {
                e.Handled = TryGoBack();
            }
        }
        #endregion

        private readonly List<(String Tag, Type Page)> _pages = new List<(String Tag, Type Page)>
        {
            ("ClipPage", typeof(ClipPage)),
            ("TranscodePage", typeof(ClipPage)), // TODO: Change this when transcode page is ready
            ("Settings", typeof(SettingsPage))
        };

    }
}
