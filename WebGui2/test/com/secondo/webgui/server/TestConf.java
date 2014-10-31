package com.secondo.webgui.server;

import com.secondo.webgui.utils.config.ConfigManager;

public class TestConf {

	public static void main(String[] args) {
		System.out.println(ConfigManager.getConfigString("IP"));
		System.out.println(ConfigManager.getConfigString("port"));
		System.out.println(ConfigManager.getConfigString("DB"));
		
	}

}
