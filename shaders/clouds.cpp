#include "pch.h"
#include "clouds.h"
#if __has_include("clouds.g.cpp")
#include "clouds.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::Pengwing::implementation
{
    clouds::clouds()
    {
        InitializeComponent();
    }

    int32_t clouds::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void clouds::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void clouds::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
