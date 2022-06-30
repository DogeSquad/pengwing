#pragma once

#include "clouds.g.h"

namespace winrt::Pengwing::implementation
{
    struct clouds : cloudsT<clouds>
    {
        clouds();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Pengwing::factory_implementation
{
    struct clouds : cloudsT<clouds, implementation::clouds>
    {
    };
}
