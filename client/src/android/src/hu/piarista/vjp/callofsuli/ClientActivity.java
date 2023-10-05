package hu.piarista.vjp.callofsuli;

import org.qtproject.qt5.android.bindings.QtActivity;
import android.os.*;
import android.content.*;
import android.app.*;

import android.view.DisplayCutout;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;

import android.content.Intent;
import android.util.Log;
import android.graphics.Rect;

public class ClientActivity extends QtActivity
{
	public static native void setUrl(String url);

	public static boolean isIntentPending;

	@Override
	public void onCreate(Bundle savedInstanceState) {
	  super.onCreate(savedInstanceState);

	  Intent theIntent = getIntent();
	  if (theIntent != null) {
		  String theAction = theIntent.getAction();
		  if (theAction != null) {
			  isIntentPending = true;
		  }
	  }

	  if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
		  getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
				  WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
		  getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
	  }
	}


	@Override
	public void onNewIntent(Intent intent) {
	  super.onNewIntent(intent);
	  setIntent(intent);

	  processIntent();
	}



	public String checkPendingIntents() {
	   if(isIntentPending) {
		   isIntentPending = false;
		   Intent intent = getIntent();

		   if (intent.getAction().equals("android.intent.action.VIEW")) {
			   String uri = intent.getDataString();
			   if (uri != null){
				   return uri;
			   }
		   }
	   }

	   return "";
   }



	private void processIntent() {
		Intent intent = getIntent();

		if (intent.getAction().equals("android.intent.action.VIEW")) {
			String uri = intent.getDataString();
			if (uri != null){
				setUrl(uri);
			}
		}
	}


	public Object getSafeArea() {
		final Rect safeInsetRect = new Rect();

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
			return safeInsetRect;
		}

		final WindowInsets windowInsets = getWindow().getDecorView().getRootWindowInsets();
		if (windowInsets == null) {
		  return safeInsetRect;
		}

		final DisplayCutout displayCutout = windowInsets.getDisplayCutout();
		if (displayCutout != null) {
		  safeInsetRect.set(displayCutout.getSafeInsetLeft(), displayCutout.getSafeInsetTop(),
		  displayCutout.getSafeInsetRight(), displayCutout.getSafeInsetBottom());
		}

		return safeInsetRect;
	 }

}
