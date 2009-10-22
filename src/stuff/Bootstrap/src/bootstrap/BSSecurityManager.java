/**
 * WebTester Server - server of on-line testing system
 *
 * Bootstrap for Java programs executing
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

package bootstrap;

import java.io.File;
import java.util.List;
import java.util.ArrayList;
import java.security.Permission;
import java.net.InetAddress;

public class BSSecurityManager extends SecurityManager
{
  private List<String> chroot;

  private List<String> explodePath (String path)
  {
    List<String> result = new ArrayList<String> ();
    String curPath = path;

    while (curPath != null && !curPath.equals ("/"))
      {
        File f = new File (curPath);

        if (!f.getName ().equals ("."))
          {
            result.add (0, f.getName ());
          }

        curPath = f.getParent ();
      }

    return result;
  }

  private boolean checkFileInsideChroot(String file)
  {
    List<String> curPath = explodePath (file);

    if (curPath.size () <= chroot.size ())
      {
        return false;
      }

    for (int i = 0; i < chroot.size (); ++i)
      {
        if (!chroot.get (i).equals (curPath.get (i)))
          {
            return false;
          }
      }

    return true;
  }

  private boolean checkFullFileInsideChroot(String file)
  {
    if (chroot == null)
      {
        return true;
      }

    File f = new File (file);
    String fullPath = f.getAbsolutePath ();

    return checkFileInsideChroot (fullPath);
  }

  public BSSecurityManager(String chroot)
  {
    File f = new File (chroot);
    this.chroot = explodePath (f.getAbsolutePath ());
  }

  @Override public void checkPermission(Permission perm)
  {
  }

  @Override public void checkRead (String file) throws SecurityException
  {
    if (!checkFullFileInsideChroot (file))
      {
        throw new SecurityException ();
      }
  }

  @Override public void checkWrite (String file) throws SecurityException
  {
    if (!checkFullFileInsideChroot (file))
      {
        throw new SecurityException ();
      }
  }

  @Override public void checkDelete (String file) throws SecurityException
  {
    if (!checkFullFileInsideChroot (file))
      {
        throw new SecurityException ();
      }
  }

  @Override public void checkLink (String file) throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkExec (String file) throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkConnect (String host, int port)
      throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkConnect (String host, int port, Object context)
      throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkListen (int port)
      throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkMulticast (InetAddress maddr)
      throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkMulticast (InetAddress maddr, byte ttl)
      throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkSetFactory () throws SecurityException
  {
    throw new SecurityException ();
  }

  @Override public void checkSystemClipboardAccess () throws SecurityException
  {
    throw new SecurityException ();
  }
}
