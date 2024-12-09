using Microsoft.Maui.Controls;

namespace WinBoost;

public partial class MainPage : ContentPage
{
    public MainPage()
    {
        InitializeComponent();
    }

    private void OnDevToolsClicked(object sender, EventArgs e)
    {
        MainContent.Content = new DevelopmentToolsPage();
    }

    private void OnGamingClicked(object sender, EventArgs e)
    {
        MainContent.Content = new GamingToolsPage();
    }

    private void OnAdvancedToolsClicked(object sender, EventArgs e)
    {
        MainContent.Content = new AdvancedToolsPage();
    }
}
