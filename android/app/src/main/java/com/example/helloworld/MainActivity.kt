// hello world
package com.example.helloworld

    import android.app.Activity
    import android.os.Bundle
    import android.webkit.WebView
    import android.webkit.WebViewClient

    class MainActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

            window.setDecorFitsSystemWindows(true)
            
            val webView: WebView = WebView(this)

            webView.settings.javaScriptEnabled = true
            webView.settings.textZoom = 100
        
            webView.webViewClient = WebViewClient()
            webView.loadUrl("file:///android_asset/index.html")

        setContentView(webView)
    }
}
