package com.pasdk;

public class PARenderer {

    static {

        System.loadLibrary("pabeauty" );
    }
    public native boolean setBeautificationOn();
    public native int onDraw(int texture_id, int width, int height);

    public native int onFilterSelected();

  //  public native int myonDrawFrame(int texture_id, int width, int height);

    public native boolean onDeleteAllFilters();

    public native boolean onDeleteOneFilter(int filter_id);


}
