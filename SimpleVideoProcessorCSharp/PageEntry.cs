// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Documents;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SimpleVideoProcessorCSharp
{
    // Code modified based on that of https://github.com/WinUICommunity/SettingsUI/blob/main/SettingsUI/Controls/SettingsGroup/SettingsGroup.cs
    public sealed class PageEntry : Control
    {
        public PageEntry()
        {
            this.DefaultStyleKey = typeof(PageEntry);
        }

        public static readonly DependencyProperty HeaderProperty = DependencyProperty.Register
        (
            nameof(Header),
            typeof(string),
            typeof(PageEntry),
            new PropertyMetadata(default(string), new PropertyChangedCallback(OnHeaderChanged))
        );
        public static readonly DependencyProperty DescriptionProperty = DependencyProperty.Register
        (
            nameof(Description),
            typeof(string),
            typeof(PageEntry),
            new PropertyMetadata(default(string), new PropertyChangedCallback(OnDescriptionChanged))
        );
        public static readonly DependencyProperty IconProperty = DependencyProperty.Register
        (
            nameof(Icon),
            typeof(object),
            typeof(PageEntry),
            new PropertyMetadata(null, new PropertyChangedCallback(OnIconChanged))
        );
        public static readonly DependencyProperty ContentProperty = DependencyProperty.Register
        (
            nameof(Content),
            typeof(object),
            typeof(PageEntry),
            new PropertyMetadata(null, new PropertyChangedCallback(OnContentChanged))
        );

        [Localizable(true)]
        public string Header
        {
            get => (string)GetValue(HeaderProperty);
            set => SetValue(HeaderProperty, value);
        }
        [Localizable(true)]
        public string Description
        {
            get => (string)GetValue(DescriptionProperty);
            set => SetValue(DescriptionProperty, value);
        }
        public object Icon
        {
            get => GetValue(IconProperty);
            set => SetValue(IconProperty, value);
        }
        public object Content
        {
            get => GetValue(ContentProperty);
            set => SetValue(ContentProperty, value);
        }

        public static void OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((PageEntry)d).Update();
        }
        public static void OnDescriptionChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((PageEntry)d).Update();
        }
        public static void OnIconChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((PageEntry)d).Update();
        }
        public static void OnContentChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ((PageEntry)d).Update();
        }

        /// <summary>
        /// Update hovered state
        /// </summary>
        /// <param name="args"></param>
        protected override void OnPointerEntered(PointerRoutedEventArgs args)
        {
            base.OnPointerEntered(args);

            IsHovered = true;
            Update();
        }
        /// <summary>
        /// Update hovered state
        /// </summary>
        /// <param name="args"></param>
        protected override void OnPointerExited(PointerRoutedEventArgs e)
        {
            base.OnPointerExited(e);

            IsHovered = false;
            Update();
        }

        /// <summary>
        /// Updates the visual appearance whenever something's changed.
        /// </summary>
        public void Update()
        {
            if(_pageEntry == null)
            {
                return;
            }

            SetEnabledState();
            SetHoveredState();

            if(_pageEntry.Content != null) 
            {
                _pageEntry._contentPresenter.Visibility = Visibility.Visible;
            }
            else
            {
                _pageEntry._contentPresenter.Visibility= Visibility.Collapsed;
            }
            if (_pageEntry.Icon != null)
            {
                _pageEntry._iconPresenter.Visibility = Visibility.Visible;
            }
            else
            {
                _pageEntry._iconPresenter.Visibility = Visibility.Collapsed;
            }

        }

        /// <summary>
        /// Sets the fields when a template is applied to this class.
        /// </summary>
        protected override void OnApplyTemplate()
        {
            _pageEntry = (PageEntry)this;

            IsEnabledChanged -= PageEntry_IsEnabledChanged;

            _iconPresenter = (ContentPresenter)_pageEntry.GetTemplateChild(PartIconPresenter);
            _contentPresenter = (ContentPresenter)_pageEntry.GetTemplateChild(PartContentPresenter);

            SetEnabledState();
            SetHoveredState();

            // adds PageEntry_IsEnabledChanged to the enabled changed handler
            IsEnabledChanged += PageEntry_IsEnabledChanged;

            base.OnApplyTemplate();
        }

        private void PageEntry_IsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            SetEnabledState();
        }

        private void SetEnabledState()
        {
            VisualStateManager.GoToState(this, IsEnabled ? "Normal" : "Disabled", true);
        }
        private void SetHoveredState()
        {
            VisualStateManager.GoToState(this, IsHovered ? "Hovered" : "Unhovered", false);
        }

        private bool IsHovered = false;

        private const string PartIconPresenter = "IconPresenter";
        private const string PartContentPresenter = "ContentPresenter";

        private ContentPresenter _iconPresenter;
        private ContentPresenter _contentPresenter;

        // Equivalent to (PageEntry)this. Set only once to avoid the overhead of casting.
        private PageEntry _pageEntry;
    }
}
