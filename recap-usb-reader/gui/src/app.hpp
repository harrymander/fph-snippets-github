#pragma once

namespace recap::app {

int SetupApp(float scaling_factor = 0);

/**
 * Returns true when the app should keep running. When returns false, should tear down GUI context,
 * close windows etc. and exit program.
 */
bool ShowApp();

void CloseApp();

void TearDownApp();

}; // namespace recap::app
