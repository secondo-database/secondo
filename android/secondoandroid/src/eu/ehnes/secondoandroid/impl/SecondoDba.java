package eu.ehnes.secondoandroid.impl;


import eu.ehnes.secondoandroid.starthelper;
import eu.ehnes.secondoandroid.itf.ISecondoDba;
import eu.ehnes.secondoandroid.itf.ISecondoDbaCallback;

public class SecondoDba implements ISecondoDba {
	
	private starthelper sh;
	private Boolean isInitialized=Boolean.FALSE;
	
	public SecondoDba() {
		sh = new starthelper();
	}

	@Override
	public boolean initializeSync(String configPath) {
		boolean result = false;
		synchronized(isInitialized) {
			if (!isInitialized.booleanValue()) {
				result = sh.initialize(configPath);
				isInitialized = Boolean.TRUE;
			}
		}
		return result;
	}

	@Override
	public Object querySync(String queryString) {
		Object result = null;
		synchronized(isInitialized) {
			if (isInitialized.booleanValue()) {
				result = sh.query(queryString);
			}
		}
		return result;
	}

	@Override
	public String errorMessageSync() {
		String result = null;
		synchronized(isInitialized) {
			if (isInitialized.booleanValue()) {
				result = sh.errorMessage();
			}
		}
		return result;
	}

	@Override
	public void shutdownSync() {
		synchronized(isInitialized) {
			if (isInitialized.booleanValue()) {
				sh.shutdown();
				isInitialized = Boolean.FALSE;
			}
		}
	}
	
	@Override
	public void queryASync(final String queryString, final ISecondoDbaCallback callBackFunction) {
		Thread t = new Thread(new Runnable() {

			@Override
			public void run() {
				Object result = querySync(queryString);
				try {
					callBackFunction.queryCallBack(result);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			
		});
		t.start();
	}
	
	@Override
	public void initializeASync(final String configPath, final ISecondoDbaCallback callBackInterface) {
		Thread t = new Thread(new Runnable() {

			@Override
			public void run() {
				boolean result = initializeSync(configPath);
				callBackInterface.initializeCallBack(result);
			}
			
		});
		t.start();
	}
}
