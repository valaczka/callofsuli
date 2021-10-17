package hu.piarista.vjp.callofsuli;

import org.qtproject.qt5.android.bindings.QtActivity;
import android.os.*;
import android.content.*;
import android.app.*;

import android.content.Intent;
import android.util.Log;

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
	}


	@Override
	public void onNewIntent(Intent intent) {
	  super.onNewIntent(intent);
	  setIntent(intent);

	  processIntent();
	}



	public boolean checkPendingIntents() {
	   if(isIntentPending) {
		   isIntentPending = false;
		   processIntent();
		   return true;
	   }

	   return false;
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
}
