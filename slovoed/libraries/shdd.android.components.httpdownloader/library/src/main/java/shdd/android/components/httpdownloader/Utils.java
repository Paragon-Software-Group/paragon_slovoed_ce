package shdd.android.components.httpdownloader;

import java.io.File;

import android.os.StatFs;

public class Utils {
	
	public static boolean hasEnoughFreeSpaceInFileSystemFor(File file, long writeSize) {
		return getAvailableBytesInFileSystemAt(file) >= writeSize;
	}
	
	public static long getAvailableBytesInFileSystemAt(File path) {
    	while (!path.isDirectory()) path = path.getParentFile();
    	StatFs stat = new StatFs(path.getAbsolutePath());
        long availableBlocks = android.os.Build.VERSION.SDK_INT >= 18 ? stat.getAvailableBlocksLong() : stat.getAvailableBlocks();
        availableBlocks += -4;/*put a bit of margin (in case creating the file grows the system by a few blocks)*/
        if (android.os.Build.VERSION.SDK_INT >= 18)
        	return stat.getBlockSizeLong() * availableBlocks;
        return stat.getBlockSize() * availableBlocks;
    }
	
    public static long getTotalBytesInFileSystemAt(File path) {
    	while (!path.isDirectory()) path = path.getParentFile();
    	StatFs stat = new StatFs(path.getAbsolutePath());
    	if (android.os.Build.VERSION.SDK_INT >= 18)
    		return stat.getBlockSizeLong() * stat.getBlockCountLong();
    	return (long)stat.getBlockSize() * stat.getBlockCount();
    }
    
}
