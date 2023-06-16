// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#define ISPROTOTYPING

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Imaging;
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
            videoInfo = new VideoInfo();
            this.InitializeComponent();
        }

        /// <summary>
        /// Update the UI elements based on the currently opened video
        /// </summary>
        private void Update()
        {
            // enable or disable elements based on if a video is opened.
            if(videoInfo.isVideoOpened)
            {
                // enable disabled elements
                ClipTimeExpander.IsEnabled = true;

                // set the texts
                VideoNameTextBlock.Text = videoInfo.filePath;
                OpenVideoEntry.Description = "A video is opened. Expand to view more info.";

                // update the preview image
                WriteableBitmap previewImage = new WriteableBitmap(previewWidth, previewHeight);
                using (Stream st = previewImage.PixelBuffer.AsStream())
                {
                    st.Write(videoInfo.previewImgData, 0, videoInfo.previewImgData.Length);
                }
                PreviewImage.Source = previewImage;
            }
            else
            {
                // disable elements
                ClipTimeExpander.IsEnabled = false;

                // set the texts
                VideoNameTextBlock.Text = "No video file is opened.";
                OpenVideoEntry.Description = "Please open a video file to proceed.";

                // update the preview image
                PreviewImage.Source = null;
            }
        }

        VideoInfo videoInfo;
        static readonly int previewWidth = 1280;
        static readonly int previewHeight = 720;

        class VideoInfo
        {
            /// <summary>
            /// Clears all data to their default values. After that, the info represents an unopened video.
            /// </summary>
            public void Clear()
            {
                isVideoOpened = false;
                filePath = "";
                previewImgData = null;
                lengthInSec = 0.0;
            }

            // The path to the opened video file.
            // Is "" iff isVideoOpened = false;
            public String filePath = "";
            // The pointer to the preview image data, managed by the internal C++ library.
            // Is null iff isVideoOpened = false; 
            public byte[] previewImgData = null;
            // Length of the video, in seconds.
            // Is 0.0 iff isVideoOpened = false; 
            public double lengthInSec = 0.0;

            public bool isVideoOpened = false;

            /// <summary>
            /// Returns fake info for prototyping.
            /// </summary>
            /// <returns>Fake video info</returns>
            public static VideoInfo getFakeVideoInfo()
            {
                VideoInfo ret = new VideoInfo();

                ret.isVideoOpened = true;
                ret.filePath = "Prototyping\\Somewhere\\somevideo.mp4";
                ret.lengthInSec = 134.5;
                ret.previewImgData = new byte[4 * previewHeight * previewWidth];
                for(int i = 0; i < previewWidth; ++i)
                {
                    for(int j = 0; j < previewHeight; ++j)
                    {
                        int colorIndex = 4 * (i * previewHeight + j);
                        ret.previewImgData[colorIndex] = (byte)(i % 128); // b
                        ret.previewImgData[colorIndex+1] = (byte)(i % 128); // g
                        ret.previewImgData[colorIndex+2] = (byte)(i % 128); // r
                        ret.previewImgData[colorIndex+3] = 127; // a
                    }
                }

                return ret;
            }
        }

        enum ClipPieceManagementState
        {
            // normal state
            Normal,
            // when delete button has been pressed and no cancellation or confirmation is given
            Deletion
        }

        ClipPieceManagementState clipPieceManagementState = ClipPieceManagementState.Normal;

        /// <summary>
        /// Updates the UI for clip pieces management based on the current state
        /// </summary>
        void UpdateClipPieceUI()
        {
            if(!videoInfo.isVideoOpened)
            {
                return;
            }

            if(clipPieceManagementState == ClipPieceManagementState.Normal)
            {
                ClipPiecesList.SelectionMode = ListViewSelectionMode.Single;

                ConfirmDeletionButton.IsEnabled = false;
                CancelDeletionButton.IsEnabled = false;

                AddClipPieceButton.IsEnabled = true;
                DeleteClipPieceButton.IsEnabled = true;

                // Is one item selected?
                if (ClipPiecesList.SelectedItem == null)
                {
                    SetStartButton.IsEnabled = false;
                    SetEndButton.IsEnabled = false;
                }
                else
                {
                    SetStartButton.IsEnabled = true;
                    SetEndButton.IsEnabled = true;
                }
            }
            else if(clipPieceManagementState == ClipPieceManagementState.Deletion)
            {
                ClipPiecesList.SelectionMode = ListViewSelectionMode.Multiple;

                AddClipPieceButton.IsEnabled = false;
                DeleteClipPieceButton.IsEnabled = false;

                ConfirmDeletionButton.IsEnabled = true;
                CancelDeletionButton.IsEnabled = true;

                SetStartButton.IsEnabled = false;
                SetEndButton.IsEnabled = false;
            }
        }

        /// <summary>
        /// Returns the time represented by the values of the input boxes
        /// </summary>
        /// <returns>The timespan parsed if the values are vaild</returns>
        /// <exception cref="InvalidDataException">
        /// If any of the value in the boxes is invalid or if the total time is greater than the video length.
        /// </exception>
        TimeSpan GetInputValue()
        {
            double hours = HourInput.Value;
            double minutes = MinuteInput.Value;
            double seconds = SecondInput.Value;

            if (hours.Equals(double.NaN))
            {
                throw new InvalidDataException("You must provide Hours.");
            }
            if (minutes.Equals(double.NaN))
            {
                throw new InvalidDataException("You must provide Minutes.");
            }
            if (seconds.Equals(double.NaN))
            {
                throw new InvalidDataException("You must provide Seconds.");
            }
            if (hours < 0.0)
            {
                throw new InvalidDataException("Hours cannot be negative.");
            }
            if (minutes < 0.0)
            {
                throw new InvalidDataException("Minutes cannot be negative.");
            }
            if (seconds < 0.0)
            {
                throw new InvalidDataException("Seconds cannot be negative.");
            }

            double totalSeconds = (long)hours * 3600.0 + (long)minutes * 60.0 + seconds;
            var ret = TimeSpan.FromSeconds(totalSeconds);
            if(ret > TimeSpan.FromSeconds(videoInfo.lengthInSec))
            {
                throw new InvalidDataException("The time you specified is greater than the length of the video");
            }
            return ret;
        }

        /// <summary>
        /// Displays a message in the clipping area to the user
        /// </summary>
        /// <param name="Title"></param>
        /// <param name="Msg"></param>
        /// <param name="severity"></param>
        void SetClipMessage(String Title, String Msg, InfoBarSeverity Severity)
        {        
            ClipMessageBar.Title = Title;
            ClipMessageBar.Message = Msg;
            ClipMessageBar.Severity = Severity;
            ClipMessageBar.IsOpen = true;
        }

        #region Button Clicks
        /// <summary>
        /// Pick up a video file and sets filePath and isVideoOpened on success
        /// </summary>
        /// <returns></returns>
#if ISPROTOTYPING
        private void OpenVideoButton_Click(object sender, RoutedEventArgs e)
#else
        private async void OpenVideoButton_Click(object sender, RoutedEventArgs e)
#endif
        {
            if (!videoInfo.isVideoOpened)
            {
                // Clear previous info, if it exists.
                videoInfo.Clear();

#if ISPROTOTYPING
                // fake video info
                videoInfo = VideoInfo.getFakeVideoInfo();
#else
                // open from a real file
                {
                    var openPicker = new Windows.Storage.Pickers.FileOpenPicker();
                    // Retrieve the window handle (HWND) of the current WinUI 3 window.
                    var window = (Application.Current as App)?.Window;
                    var hWnd = WinRT.Interop.WindowNative.GetWindowHandle(window);

                    // Initialize the file picker with the window handle (HWND).
                    WinRT.Interop.InitializeWithWindow.Initialize(openPicker, hWnd);

                    // Set options for the file picker
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
                        // Update video info.
                        // TODO: Call functions from the internal library to complete the info.
                        videoInfo.isVideoOpened = true;
                        videoInfo.filePath = file.Path;
                    }
                    else
                    {
                        // TODO: Show an error message somewher.
                    }
                }
#endif

                Update();
                UpdateClipPieceUI();
            }
        }

        private void ClearVideoButton_Click(object sender, RoutedEventArgs e)
        {
            if (videoInfo.isVideoOpened)
            {
                videoInfo.Clear();

                Update();
            }
        }

        private void StartClippingButton_Click(object sender, RoutedEventArgs e)
        {
#if ISPROTOTYPING

#else

#endif
        }
        #endregion

        private void SetStartButton_Click(object sender, RoutedEventArgs e)
        {
            if(ClipPiecesList.SelectedItem == null)
            {
                // Actually cannot happen because then the buttom would be disabled
                return;
            }

            TimeSpan time = TimeSpan.Zero;
            try
            {
                time = GetInputValue();
            }
            catch (InvalidDataException ex) 
            {
                SetClipMessage("Input Error", ex.Message, InfoBarSeverity.Error);
                return;
            }

            // update the selected item
            int ind = ClipPiecesList.SelectedIndex;
            var cp = (ClipPiece)ClipPiecesList.Items[ind];
            if(time >= cp.GetEndTimeSpan())
            {
                SetClipMessage("Input Error", "Start must < End", InfoBarSeverity.Error);
                return;
            }
            cp.SetStartFromTimeSpan(time);
            ClipPiecesList.Items[ind] = cp;
            ClipPiecesList.SelectedIndex = ind;

            // succeed. close the error message
            ClipMessageBar.IsOpen = false;
        }

        private void SetEndButton_Click(object sender, RoutedEventArgs e)
        {
            if (ClipPiecesList.SelectedItem == null)
            {
                // Actually cannot happen because then the buttom would be disabled
                return;
            }

            TimeSpan time = TimeSpan.Zero;
            try
            {
                time = GetInputValue();
            }
            catch (InvalidDataException ex)
            {
                SetClipMessage("Input Error", ex.Message, InfoBarSeverity.Error);
                return;
            }

            // update the selected item
            int ind = ClipPiecesList.SelectedIndex;
            var cp = (ClipPiece)ClipPiecesList.Items[ind];
            if (time <= cp.GetStartTimeSpan())
            {
                SetClipMessage("Input Error", "Start must < End", InfoBarSeverity.Error);
                return;
            }
            cp.SetEndFromTimeSpan(time);
            ClipPiecesList.Items[ind] = cp;
            ClipPiecesList.SelectedIndex = ind;

            // succeed. close the error message
            ClipMessageBar.IsOpen = false;
        }

        private void ConfirmDeletionYesButton_Click(object sender, RoutedEventArgs e)
        {
            if (clipPieceManagementState != ClipPieceManagementState.Deletion)
            {
                return;
            }

            while(ClipPiecesList.SelectedIndex != -1)
            {
                ClipPiecesList.Items.RemoveAt(ClipPiecesList.SelectedIndex);
            }

            clipPieceManagementState = ClipPieceManagementState.Normal;
            DeletionConfirmationFlyout.Hide();
            UpdateClipPieceUI();
        }

        private void AddClipPieceButton_Click(object sender, RoutedEventArgs e)
        {
            // Adds an empty clip piece
            ClipPiecesList.Items.Add(new ClipPiece());

            UpdateClipPieceUI();
        }

        private void DeleteClipPieceButton_Click(object sender, RoutedEventArgs e)
        {
            if(clipPieceManagementState == ClipPieceManagementState.Deletion) 
            {
                // Already in deletion state
                return;
            }

            if(ClipPiecesList.Items.Count > 0) // if there are items
            {
                clipPieceManagementState = ClipPieceManagementState.Deletion;
                UpdateClipPieceUI();
                ClipMessageBar.IsOpen = false;
            }
            else
            {
                SetClipMessage("No Item", "No Item to delete.", InfoBarSeverity.Warning);
            }
        }

        private void CancelDeletionButton_Click(object sender, RoutedEventArgs e)
        {
            if(clipPieceManagementState == ClipPieceManagementState.Normal)
            {
                return;
            }

            clipPieceManagementState = ClipPieceManagementState.Normal;
            UpdateClipPieceUI();
        }

        private void ClipPiecesList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            UpdateClipPieceUI();
        }
    }
}
