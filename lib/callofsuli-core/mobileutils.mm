/*
 * ---- Call of Suli ----
 *
 * mobileutils.cpp
 *
 * Created on: 2023. 07. 05.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MobileUtils
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "qurl.h"

#include <AudioToolbox/AudioToolbox.h>
#include <UIKit/UIKit.h>

#include "mobileutils.h"
#include "Logger.h"
#include "application.h"


MobileUtils* MobileUtils::m_instance = nullptr;


@interface AppDelegate : UIResponder <UIApplicationDelegate>
						 +(AppDelegate *)sharedAppDelegate;
@end


@implementation AppDelegate
static AppDelegate *appDelegate = nil;

+(AppDelegate *)sharedAppDelegate
{
	static dispatch_once_t pred;
	static AppDelegate *shared = nil;
	dispatch_once(&pred, ^{
					  shared = [[super alloc] init];
				  });
	return shared;
}

void MobileUtils::initialize()
{
	//get app delegate.
	AppDelegate *appDelegate = (AppDelegate *)[[UIApplication sharedApplication] delegate];
	[[UIApplication sharedApplication] setDelegate:[AppDelegate sharedAppDelegate]];
	LOG_CDEBUG("utils") << "Created new AppDelegate";
}

- (BOOL)application:(UIApplication *)app
  openURL:(NSURL *)url
  options:(NSDictionary<UIApplicationOpenURLOptionsKey,id> *)options
{
	id sendingAppID = [options objectForKey: UIApplicationOpenURLOptionsSourceApplicationKey];

	if (sendingAppID)
		NSLog(@"source application = %@", sendingAppID);

	NSURLComponents *components = [[NSURLComponents alloc] initWithURL: url resolvingAgainstBaseURL: YES];

	if (!components) {
		LOG_CDEBUG("utils") << "Invalid URL";
		return NO;
	}

	NSString *myString = url.absoluteString;

	std::string tmpString
			(
				[myString cStringUsingEncoding: NSUTF8StringEncoding],
			[myString lengthOfBytesUsingEncoding: NSUTF8StringEncoding]
			);

	MobileUtils::openUrl(tmpString);

	return YES;
}

@end



/**
 * @brief MobileUtils::MobileUtils
 */

MobileUtils::MobileUtils()
{

}


/**
 * @brief MobileUtils::vibrate
 */

void MobileUtils::vibrate(const int &)
{
	LOG_CDEBUG("utils") << "Call iOS Vibrator";
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}


/**
 * @brief MobileUtils::openUrl
 * @param url
 */

void MobileUtils::openUrl(const std::string &url)
{
	QUrl _url(QString::fromStdString(url));
	Application::instance()->selectUrl(_url);
}


QString MobileUtils::checkPendingIntents()
{
	return QLatin1String("");
}

