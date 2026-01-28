package hu.piarista.vjp.callofsuli;

import org.qtproject.qt.android.bindings.QtActivity;
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

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
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



