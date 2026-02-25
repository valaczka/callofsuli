package hu.piarista.vjp.callofsuli;

import org.qtproject.qt.android.bindings.QtActivity;
import android.os.*;
import android.content.*;
import android.app.*;

import android.content.res.Configuration;

import android.view.DisplayCutout;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;


import android.content.Intent;
import android.util.Log;
import android.graphics.Rect;
import android.graphics.Insets;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import java.security.MessageDigest;
import java.util.ArrayList;


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

	  enterImmersive();
	}


	@Override
	public void onNewIntent(Intent intent) {
	  super.onNewIntent(intent);
	  setIntent(intent);

	  processIntent();
	}


	@Override
	protected void onResume() {
		super.onResume();

		forceQtViewFullBleed();
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			forceQtViewFullBleed();
		}
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);

		forceQtViewFullBleed();
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



	public void enterImmersive() {
		View qtView = getWindow().getDecorView();

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
					WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
			getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
		}

		if (Build.VERSION.SDK_INT >= 28 && Build.VERSION.SDK_INT < 30) {
			qtView.setOnApplyWindowInsetsListener((v, insets) -> {
				WindowInsets out = insets.consumeSystemWindowInsets();
				out = out.consumeDisplayCutout();
				return out;
			});
		} else if (Build.VERSION.SDK_INT >= 30) {
			qtView.setFitsSystemWindows(false);
			qtView.setOnApplyWindowInsetsListener((v, insets) -> insets);
		}
	}


	private void reloadLayout() {
		View qtView = getWindow().getDecorView();

		qtView.requestApplyInsets();
		qtView.requestLayout();
	}


	public void forceQtViewFullBleed() {
		getWindow().getDecorView().postDelayed(this::reloadLayout, 50);
		getWindow().getDecorView().postDelayed(this::reloadLayout, 100);
		getWindow().getDecorView().postDelayed(this::reloadLayout, 500);
		getWindow().getDecorView().postDelayed(this::reloadLayout, 750);
		getWindow().getDecorView().postDelayed(this::reloadLayout, 1000);
		getWindow().getDecorView().postDelayed(this::reloadLayout, 2000);
	}



	public Object getSafeArea() {
		final Window w = getWindow();
		final View decor = w.getDecorView();
		final Rect safeInsetRect = new Rect();

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
			return safeInsetRect;
		}

		if (Build.VERSION.SDK_INT >= 30) {
			 WindowInsets insets = decor.getRootWindowInsets();
			 if (insets != null) {
				 // System bars (status + nav)
				 Insets sb = insets.getInsets(
						 WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars()
				 );


				 // Display cutout (notch)
				 Insets cut = insets.getInsets(WindowInsets.Type.displayCutout());

				 safeInsetRect.set(
					 Math.max(sb.left, cut.left),
					 Math.max(sb.top, cut.top),
					 Math.max(sb.right, cut.right),
					 Math.max(sb.bottom, cut.bottom)
				 );
			 }

		  return safeInsetRect;
		}

		final WindowInsets windowInsets = decor.getRootWindowInsets();

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


	 // Returns list of SHA-256 fingerprints (hex, uppercase, no colons)
	 public String[] getSigningCertSha256() throws Exception {
		 PackageManager pm = getPackageManager();
		 String pkg = getPackageName();

		 ArrayList<String> out = new ArrayList<>();

		 if (Build.VERSION.SDK_INT >= 28) {
			 PackageInfo pi = pm.getPackageInfo(pkg, PackageManager.GET_SIGNING_CERTIFICATES);
			 if (pi.signingInfo != null) {
				 Signature[] sigs;
				 // If key rotation happened, getSigningCertificateHistory() may contain old certs too.
				 if (pi.signingInfo.hasMultipleSigners()) {
					 sigs = pi.signingInfo.getApkContentsSigners();
				 } else {
					 sigs = pi.signingInfo.getSigningCertificateHistory();
				 }
				 if (sigs != null) {
					 for (Signature s : sigs) {
						 out.add(sha256Hex(s.toByteArray()));
					 }
				 }
			 }
		 } else {
			 @SuppressWarnings("deprecation")
			 PackageInfo pi = pm.getPackageInfo(pkg, PackageManager.GET_SIGNATURES);
			 @SuppressWarnings("deprecation")
			 Signature[] sigs = pi.signatures;
			 if (sigs != null) {
				 for (Signature s : sigs) {
					 out.add(sha256Hex(s.toByteArray()));
				 }
			 }
		 }

		 return out.toArray(new String[0]);
	 }

	 private String sha256Hex(byte[] data) throws Exception {
		 MessageDigest md = MessageDigest.getInstance("SHA-256");
		 byte[] dig = md.digest(data);

		 // hex uppercase, no separators
		 final char[] HEX = "0123456789ABCDEF".toCharArray();
		 char[] chars = new char[dig.length * 2];
		 for (int i = 0; i < dig.length; i++) {
			 int v = dig[i] & 0xFF;
			 chars[i * 2]     = HEX[v >>> 4];
			 chars[i * 2 + 1] = HEX[v & 0x0F];
		 }
		 return new String(chars);
	 }

}



