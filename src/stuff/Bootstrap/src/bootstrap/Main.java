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

import java.util.List;
import java.util.ArrayList;

import java.io.File;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;

public class Main
{
  private String mainClassName = "Main";
  private static Main instance = null;
  private List<String> jars;
  private List<String> classes;
  private String[] childArgs;

  private String chroot = "";

  private ClassLoader classLoader;
  private Class mainClass;
  private Method mainMethod;

  private static final int STATE_READING_ARGS = 0;
  private static final int STATE_READING_JARS = 1;
  private static final int STATE_READING_CLASSES = 2;

  private Main ()
  {
    jars = new ArrayList<String> ();
    classes = new ArrayList<String> ();
  }

  private static Main getInstance ()
  {
    if (instance == null)
      {
        instance = new Main ();
      }

    return instance;
  }

  private boolean isValidOpt (String opt)
  {
    if (opt.equals ("-jars") || opt.equals ("-classes-dirs") ||
        opt.equals ("-chroot") || opt.equals ("-main-class"))
      {
        return true;
      }

    return false;
  }

  private void parseArgs (String[] args)
  {
    int i = 0, n = args.length;
    int state = STATE_READING_ARGS;

    while (i < n)
      {
        switch (state)
          {
          case STATE_READING_ARGS:
            if (args[i].equals ("-jars"))
              {
                state = STATE_READING_JARS;
                ++i;
              }
            else if (args[i].equals ("-classes-dirs"))
              {
                state = STATE_READING_CLASSES;
                ++i;
              }
            else if (args[i].equals ("-chroot"))
              {
                chroot = args[i + 1];
                i += 2;
              }
            else if (args[i].equals ("-main-class"))
              {
                mainClassName = args[i + 1];
                i += 2;
              }
            else
              {
                String[] newChildArgs = new String[childArgs.length + 1];

                for (int j = 0; j < childArgs.length; ++j)
                  {
                    newChildArgs[j] = childArgs[j];
                  }

                childArgs[childArgs.length] = args[i];
            }
            break;

          case STATE_READING_JARS:
          case STATE_READING_CLASSES:
            if (isValidOpt (args[i]))
              {
                /* If current argument is a valid argument option */
                /* return to arguments reading */
                state = STATE_READING_ARGS;
              }
            else
            {
              if (state == STATE_READING_JARS)
                {
                  /* Add file to list of jars to load */
                  jars.add (args[i++]);
                }
              else
                {
                  /* Add file to list of classes to load */
                  classes.add (args[i++]);
                }
            }
            break;
          }
      }
  }

  private URL[] getClassesURLs () throws Exception
  {
    URL[] result;
    int i = 0;

    /* Allocate memory for URL list */
    result = new URL[classes.size () + jars.size ()];

    for (String c : classes) {
      result[i++] = new File (c).toURI ().toURL ();
    }

    for (String c : jars) {
      result[i++] = new File (c).toURI ().toURL ();
    }

    return result;
  }

  private void prepareClassLoader () throws Exception
  {
    URL[] urls = getClassesURLs ();

    classLoader = new URLClassLoader (urls,
            ClassLoader.getSystemClassLoader ().getParent ());
  }

  private void loadEntryPoint (Class argClass) throws Exception
  {
    mainClass = classLoader.loadClass (mainClassName);

    mainMethod = mainClass.getMethod ("main", new Class[] { argClass });
  }

  private void runChild (String[] args) throws Exception
  {
    Thread.currentThread ().setContextClassLoader (classLoader);

    System.setSecurityManager (new BSSecurityManager (chroot));

    mainMethod.invoke (null, new Object[] {args});
  }

  private void run (String[] args) throws Exception
  {
    /* Parse command line arguments */
    parseArgs (args);

    /* Prepare class loader */
    prepareClassLoader ();

    /* Load main class and program entry point */
    loadEntryPoint (args.getClass ());

    /* Run child class */
    runChild (childArgs);
  }

  public static void main(String[] args) throws Exception
  {
    Main.getInstance ().run (args);
  }
}
