// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage.Pickers;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SimpleVideoProcessorCSharp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class ClipPage : Page
    {
        public ClipPage()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Pick up a video file and sets filePath and isVideoOpened on success
        /// </summary>
        /// <returns></returns>
        private async void OpenVideoButton_Click(object sender, RoutedEventArgs e)
        {
            // Clear previous returned file name, if it exists.
            if(!isVideoOpened)
            {
                filePath = "";

                {
                    var openPicker = new Windows.Storage.Pickers.FileOpenPicker();
                    // Retrieve the window handle (HWND) of the current WinUI 3 window.
                    var window = (Application.Current as App)?.Window;
                    var hWnd = WinRT.Interop.WindowNative.GetWindowHandle(window);

                    // Initialize the file picker with the window handle (HWND).
                    WinRT.Interop.InitializeWithWindow.Initialize(openPicker, hWnd);

                    // Set options for your file picker
                    openPicker.ViewMode = PickerViewMode.Thumbnail;
                    openPicker.SuggestedStartLocation = PickerLocationId.VideosLibrary;
                    openPicker.FileTypeFilter.Add(".mp4");
                    openPicker.FileTypeFilter.Add(".mkv");
                    openPicker.FileTypeFilter.Add(".wmv");
                    openPicker.FileTypeFilter.Add(".flv");
                    openPicker.FileTypeFilter.Add(".mp3");

                    // Open the picker for the user to pick a file
                    var file = await openPicker.PickSingleFileAsync();
                    if (file != null)
                    {
                        isVideoOpened = true;
                        filePath = file.Path;
                    }
                    else
                    {
                    }
                }

                Update();
            }
        }

        /// <summary>
        /// Update the visual appearances
        /// </summary>
        private void Update()
        {
            // enable or disable elements based on if a video is opened.
            if(isVideoOpened)
            {
                ClipTimeExpander.IsEnabled = true;

                // also set the texts
                VideoNameTextBlock.Text = filePath;
                OpenVideoEntry.Description = "A video is opened. Expand to view more info.";
            }
            else
            {
                ClipTimeExpander.IsEnabled = false;

                // also set the texts of the file
                VideoNameTextBlock.Text = "No video file is opened.";
                OpenVideoEntry.Description = "Please open a video file to proceed.";
            }
        }

        private String filePath = "";
        private bool isVideoOpened = false;

        private void ClearVideoButton_Click(object sender, RoutedEventArgs e)
        {
            if(isVideoOpened)
            {
                isVideoOpened = false;
                filePath = "";

                Update();
            }
        }
    }
}
