Lunar Trash Panel Applet (lunar-tpa)
======================================

Lunar-tpa is an extension for the Expidus Panel, which enables users to add a trash can to their panel, that

 * displays the current state of the trash can
 * can move files to the trash by dropping them on the trash icon
 * can empty the trash can
 * can open the trash can

In order to build and install this plugin, you will need to have the expidus1-panel development headers and libraries installed (the appropriate package is usually called expidus1-panel-dev or expidus1-panel-devel). In addition, you'll need to have D-BUS 0.34 or above installed and Lunar must be built with D-BUS support.


How does it work?
=================

To avoid running several desktop processes that all monitor and manage the trash can by itself, and thereby create an unnecessary maintaince and resource overhead, the trash applet simply connects to Lunar via D-BUS to query the state of the trash and send commands to the Trash.

The trash applet is not limited to Lunar, but can work with any file manager that implements the com.expidus.Trash interface and owns the com.expidus.FileManager service. See the lunar-tpa bindings.xml file for a details about the inter- face.

