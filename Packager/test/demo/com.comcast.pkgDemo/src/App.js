import { Lightning, Utils } from 'wpe-lightning-sdk'

import beautify  from 'json-beautify'
import ThunderJS from 'ThunderJS'
import Events    from './components/Events'
import AppList   from "./components/AppList";
import OkCancel  from "./components/OkCancel";

import { DefaultApps as inventory, DefaultApps } from "./DefaultApps.js";

var AvailableApps = [];
var InstalledApps = [];

var InstalledAppMap = {};

const thunder_cfg = {
  host: '127.0.0.1',
  port: 9999,
  debug: false, // VERY USEFUL
  versions: {
    default: 1, // use version 5 if plugin not specified
    Controller: 1,
    Packager: 1,
    // etc ..
  }
}

var thunderJS = null;

export default class App extends Lightning.Component
{
  static getFonts() {
    return [{ family: 'Regular', url: Utils.asset('fonts/Roboto-Regular.ttf') }]
  }

  static _template()
  {
    let RR = Lightning.shaders.RoundedRectangle;

    var ui =
    {
      Blind1: {
        x: 0, y: 0, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 998,

        // Bg: {mountX: 0.5,
        //   x: 1920/2, y: 1080/2, w: 600, h: 79, rect: true, color: 0xff000000, zIndex: 998,
        // },

        RDKlogo: {
          mount: 0.5,
          x: 1920/2,
          y: 1080/2,
          zIndex: 999,
          src: Utils.asset('images/RDKLogo400x79.png'),
        },
      },
      Blind2: {
        x: 0, y: 1080/2, w: 1920, h: 1080/2, rect: true, color: 0xff000000, zIndex: 997
      },


      Background: {
        w: 1920,
        h: 1080,
        color: 0xff8888aa,
        src: Utils.asset('images/background1.png'),
      },


      Title: {
        mountX: 0.5,
        mountY: 0,
        x: 1920/2,
        y: 20,
        text: {
          text: "Demo Store",
          fontFace: 'Regular',
          fontSize: 70,
          textColor: 0xFFffffff,

          shadow: true,
          shadowColor: 0xff000000,
          shadowOffsetX: 2,
          shadowOffsetY: 2,
          shadowBlur: 8,
        },
      },

      Lists:
      {
        mountX: 0.5, x: 1920/2, y: 150, w: 1450, h: 900,
        flex: {direction: 'column', alignItems: 'center'},
      //	rect: false, //rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,
        // rect: true, color: 0x88ccccFF,

        HelpText:
        {
          // rect: true, color: 0xff00ff00,

          flex: {direction: 'row'},

          HelpBox1:
          {
            w: 1450/2,
            h: 10,
            // rect: true, color: 0xffFF0000,

            HelpTip1:
            {
              text:
              {
                w: 1450/2,
                text: "Use  (A)ll or (I)nfo for package metadata",
                textAlign: 'center',
                fontFace: 'Regular',
                fontSize: 16,
                textColor: 0xFFffffff,

                shadow: true,
                shadowColor: 0xff000000,
                shadowOffsetX: 2,
                shadowOffsetY: 2,
                shadowBlur: 8,
              },
            },
          },//Box
          HelpBox2:
          {
            w: 1450/2,
            h: 10,
            // rect: true, color: 0x88FF00FF,

            HelpTip2:
            {
              text:
              {
                w: 1450/2,
                // h: 50,
                text: "Use  UP/DN  arrow keys for Console",
                textAlign: 'center',
                fontFace: 'Regular',
                fontSize: 16,
                textColor: 0xFFffffff,

                shadow: true,
                shadowColor: 0xff000000,
                shadowOffsetX: 2,
                shadowOffsetY: 2,
                shadowBlur: 8,
              },
            },
          },//Box
        }, // HelpText

        AvailableTXT: {
          // rect: true, color: 0x88FF00FF,
          x: 0,
          y: 60,
          w: 1450,
          text: {
            text: "AVAILABLE: ",

            // highlight: true,
            // highlightColor: 0xFF0000ff,

            // textAlign: 'left',
            fontFace: 'Regular',
            fontSize: 30,
            textColor: 0xFFffffff,

            shadow: true,
            shadowColor: 0xff000000,
            shadowOffsetX: 2,
            shadowOffsetY: 2,
            shadowBlur: 8,
          },
        },

        AvailableGroup:
        {
          mountX: 0.5, x: 1450/2, y: 0, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: false }, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

          // Available PACKAGES from inventory ... injected here
          AvailableList: { x: 0, type: AppList }
        },


        InstalledTXT: {
          // rect: true, color: 0x88FF00FF,
          x: 0,
          y: 160,
          w: 1450,
          text: {
            text: "INSTALLED: ",

            // highlight: true,
            // highlightColor: 0xFF0000ff,

            // textAlign: 'left',
            fontFace: 'Regular',
            fontSize: 30,
            textColor: 0xFFffffff,

            shadow: true,
            shadowColor: 0xff000000,
            shadowOffsetX: 2,
            shadowOffsetY: 2,
            shadowBlur: 8,
          },
        },
        InstalledGroup:
        {
          mountX: 0.5, x: 1450/2, y: 100, w: 1450, h: 300, flex: {direction: 'row', padding: 15, wrap: true}, rect: true, rtt: true, shader: { radius: 20, type: RR}, color: 0x4F888888,

          InstalledList: { type: AppList }

        }, // InstalledGroup
      },//Lists

      SpaceLeft:
      {
        x: 1400, y: 600,
        text: {
          text: "Space: 0 Kb",
          textAlign: 'right',
          fontFace: 'Regular',
          fontSize: 22,
          textColor: 0xaa00FF00,

          shadow: true,
          shadowColor: 0xFF000000,
          shadowOffsetX: 2,
          shadowOffsetY: 2,
          shadowBlur: 8,
        },

      },

      ConsoleBG:
      {
        mountX: 0.5,
        x: 1920/2, y: 150, w: 1450,
        h: 600, rect: true,
        alpha: 0.0, shader: { radius: 20, type: RR },
        color: 0xcc222222, // #222222ee
        // colorTop: 0xFF636EFB, colorBottom: 0xFF1C27bC,

        Console: {

          x: 10, y: 10,
          w: 1450,
          //h: 500,
          text: {
            fontFace: 'Regular',
            fontSize: 18,
            textColor: 0xFFffffff,
          },
        },
      }, // ConsoleBG

      // InfoButton: {
      //   w: 60,
      //   h: 60,

      //   src: Utils.asset('images/info.png'),
      // },

      OkCancel: { type: OkCancel, x: 1920/2, y: 400, w: 600, h: 180, alpha: 0.0 },

      // LineH: { mountY: 0.5, x: 0, y: 1080/2, w: 1920, h: 2, rect: true, color: 0xff00FF00 },
      // LineV: { mountX: 0.5, y: 0, x: 1920/2, h: 1080, w: 2, rect: true, color: 0xff00FF00 },
    };

    return ui;
  }

  setConsole(str)
  {
    this.tag('Console').text.text = str;
  }

  $onRemoveOK() // 'okButton = true' indicates the OK button was clicked
  {
    var dlg = this.tag("OkCancel");
    var pkg_id = dlg.pkgId;

    console.log("onRemoveOK ENTER - ... pkg_id: " + pkg_id);

    if(pkg_id == undefined)
    {
      console.log("onRemoveOK() >>>  ERROR - ... pkg_id: " + pkg_id)
      return;
    }

    this.removePkg(pkg_id);

    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

    var button = this.tag('InstalledList').children[this.installedButtonIndex];
    button.stopWiggle();

    this._setState('InstalledRowState');
}

  $onRemoveCANCEL()
  {
    var dlg = this.tag("OkCancel");
    var pkg_id = dlg.pkgId;

    console.log("onRemoveCANCEL ENTER - ... pkg_id: " + pkg_id);

    dlg.setSmooth('alpha', 0, {duration: 0.3}); // HIDE

    console.log("onRemoveCANCEL ENTER - ... info: " + pkg_id)

    var button = this.tag('InstalledList').children[this.installedButtonIndex];
    button.stopWiggle();

    this._setState('InstalledRowState');
  }

  $InstallClicked(pkg_id)
  {
    console.log("$InstallClicked() >>>  ENTER")
    console.log("$InstallClicked() >>>  ENTER")
    console.log("$InstallClicked() >>>  ENTER")

    console.log("INSTALL >>  InstallClicked() - ENTER .. pkg_id: " + pkg_id);

    var button = this.tag('AvailableList').children[this.storeButtonIndex];

    // console.log("INSTALL >>  isInstalled: " + button.isInstalled())

    this.isInstalled(pkg_id).then( (ans) =>
    {
      if( ans['available'] == "false")
      {
        var info = button.info;

        console.log("CALL >> this.installPkg() ... info: " + info)

        this.installPkg(pkg_id, info);
      }
    });
  }

  $LaunchClicked(pkg_id)
  {
    console.log("LaunchClicked() >>>  ENTER - ... pkg_id: " + pkg_id)

    let info = InstalledAppMap[pkg_id];
    if(info) //button.isInstalled() == true)
    {
      console.log("LaunchClicked Call >> launchPkg() ... info: " + info)

      this.launchPkg(pkg_id, info);
    }
  }

  async getAvailableSpace()
  {
    var result = await thunderJS.call('Packager', 'getAvailableSpace', null);

    //this.setConsole( beautify(result, null, 2, 100) )

    this.tag('SpaceLeft').text.text = ("Space Remaining: " + result.availableSpaceInKB + " Kb");
  }

  async getPackageInfo(pkg_id)
  {
    let info  = { "pkgId": pkg_id };

    var result = await thunderJS.call('Packager', 'getPackageInfo', info);

    // console.log('Called >>  RESULT: ' + JSON.stringify(result));

    this.setConsole( beautify(result, null, 2, 100) )
  }

  async getInstalled()
  {
    console.log("getInstalled() - ENTER ")

    var result = await thunderJS.call('Packager', 'getInstalled', null);

    this.setConsole( beautify(result, null, 2, 100) )

    this.getAvailableSpace();

    InstalledAppMap = {}    // reset
    InstalledApps   = null; // reset

    result.applications.map( (o) => InstalledAppMap[o.pkgId] = o ); // populate

    InstalledApps = result.applications; // update array

    this.tag("InstalledList").children.map( (t, i) =>
    {
      if(i < InstalledApps.length)
      {
        InstalledApps[i].pkgInstalled = true;

        console.log("getInstalled() -     pkdId: " + InstalledApps[i].id)
       // console.log("getInstalled() - installed: " + InstalledApps[i].installed)

        t.info = InstalledApps[i]
        t.show(i * 0.15);
      }
      else
      {
        t.info = null;
        t.hide();
      }
    });
  }

  async isInstalled(pkd_id)
  {
    let result = await thunderJS.call('Packager', 'isInstalled', pkd_id);

    this.setConsole( beautify(result, null, 2, 100) )

    console.log('isInstalled() ... result: ' +  beautify(result, null, 2, 100) );

    return result;
  }

  async launchPkg(pkg_id, info)
  {
    console.log("launchPkg ENTER - ... pkg_id: " + pkg_id)
    console.log("launchPkg ENTER - ... info: " +  info.bundlePath)

    let params =
    {
        "client": pkg_id,
        "uri": pkg_id, //TODO:  Unexpected... check why ...  // info.bundlePath,
        "mimeType": "application/dac.native"
    }

    var result = await thunderJS.call('org.rdk.RDKShell.1', 'launchApplication', params);

    // console.log('installPkg() >>> Called >>  RESULT: ' + JSON.stringify(result));

    this.setConsole( beautify(result, null, 2, 100) )
  }


  async installPkg(thisPkgId, info)
  {
    var myEvents = new Events(thunderJS, thisPkgId);

    let buttons  = this.tag('AvailableList').children
    let button   = buttons[this.storeButtonIndex];
    let progress = button.tag('Progress')

    progress.reset(); // reset

    let handleFailure = (notification, str) =>
    {
      console.log("FAILURE >> '"+str+"' ... notification = " + JSON.stringify(notification) )

      if(thisPkgId == notification.pkgId)
      {
        button.setIcon(Utils.asset('images/x_mark.png'))

        progress.setSmooth('alpha', 0, {duration: 1.3});

        setTimeout( () =>
        {
          button.setIcon(Utils.asset('images/x_mark.png'))

          progress.reset(); // reset

          this.getAvailableSpace()

        }, 1.2 * 1000); //ms

        this.setConsole( beautify(notification, null, 2, 100) )
      }
    }

    let handleFailureDownload     = (notification) => { handleFailure(notification,'FailureDownload')     };
    let handleFailureDecryption   = (notification) => { handleFailure(notification,'FailureDecryption')   };
    let handleFailureExtraction   = (notification) => { handleFailure(notification,'FailureExtraction')   };
    let handleFailureVerification = (notification) => { handleFailure(notification,'FailureVerification') };
    let handleFailureInstall      = (notification) => { handleFailure(notification,'FailureInstall')      };

    let handleProgress = (notification) =>
    {
      // console.log("HANDLER >> pkgId: "+thisPkgId+" ... notification = " + JSON.stringify(notification) );

      if(thisPkgId == notification.pkgId)
      {
        let pc = notification.status / 8.0;
        progress.setProgress(pc);

        console.log("HANDLER >> pkgId: "+thisPkgId+" ... progress = " + pc );

        if(pc == 1.0)
        {
          progress.setSmooth('alpha', 0, {duration: 2.3});

          var ans = AvailableApps.filter( (o) => { return o.pkgId == notification.pkgId; });

          if(ans.length == 1)
          {
            var info = ans[0];
            this.onPkgInstalled(info)

            if(info.events)
            {
              info.events.disposeAll(); // remove event handlers
              info.events = null;
            }
          }
        }//ENDIF - 100%
      }
    }

    myEvents.add( 'Packager', 'onDownloadCommence', handleProgress);
    myEvents.add( 'Packager', 'onDownloadComplete', handleProgress);

    myEvents.add( 'Packager', 'onExtractCommence',  handleProgress);
    myEvents.add( 'Packager', 'onExtractComplete',  handleProgress);

    myEvents.add( 'Packager', 'onInstallCommence',  handleProgress);
    myEvents.add( 'Packager', 'onInstallComplete',  handleProgress);

    myEvents.add( 'Packager', 'onDownload_FAILED',     handleFailureDownload,) ;
    myEvents.add( 'Packager', 'onDecryption_FAILED',   handleFailureDecryption) ;
    myEvents.add( 'Packager', 'onExtraction_FAILED',   handleFailureExtraction) ;
    myEvents.add( 'Packager', 'onVerification_FAILED', handleFailureVerification);
    myEvents.add( 'Packager', 'onInstall_FAILED',      handleFailureInstall);

    var result = await thunderJS.call('Packager', 'install', info);

    info.events = myEvents
    // console.log('Called >>  RESULT: ' + JSON.stringify(result));

    this.setConsole( beautify(result, null, 2, 100) )
  }

  async removePkg(pkg_id)
  {
    console.log("removePkg() >>>    ENTER - ... pkg_id: " + pkg_id)

    if(pkg_id == undefined)
    {
      console.log("removePkg() >>>  ERROR - ... pkg_id: " + pkg_id)
      return;
    }

    var params = {
      "pkgId": pkg_id
    }

    var result = await thunderJS.call('Packager', 'remove', params);

    console.log('Called >> Remove() ... RESULT: ' + JSON.stringify(result));
    this.setConsole( beautify(result, null, 2, 100) )

    // let buttons = this.tag('AvailableList').children
    // let button  = buttons[this.installedButtonIndex];

    // button.setIcon(Utils.asset('images/download3.png'))

    // Update the Installed

    this.getAvailableSpace()
    this.getInstalled();
  }

  onPkgInstalled(info)
  {
    console.log('onPkgInstalled() ... Installed >>> ' + info.pkgId)

    // console.log('onPkgInstalled() ... children >>> ' + this.tag('InstalledList').children.length);

    info.pkgInstalled = true;

    InstalledApps.push( info )
    InstalledAppMap[info.pkgId] = info; // populate

    this.tag('InstalledList').addTile(InstalledApps.length - 1, info)

    this.getAvailableSpace()
  }

  _init()
  {
    this.storeButtonIndex     = 0;
    this.installedButtonIndex = 0;

    this.tag('Background').on('txLoaded', () =>
    {
      this._setState('IntroState');
    });
  }

  handleToggleConsole()
  {
    let a = this.tag("ConsoleBG").alpha;
    this.tag("ConsoleBG").setSmooth('alpha', (a == 1) ? 0 : 1, {duration: 0.3});
  }

  handleGetInfoALL()
  {
    this.getInstalled();
  }

  handleGetInfo()
  {
    let info = InstalledApps[this.storeButtonIndex];

    this.getPackageInfo(info.pkgId || info.id);
  }

  // GLOBAL key handling
  _handleKey(k)
  {
    switch( k.keyCode )
    {
      case 65:  // 'A' key on keyboard
      case 403: // 'A' key on remote
          this.handleGetInfoALL();
          break;

      case 67:  // 'C' key on keyboard
      case 405: // 'C' key on remote
          this.handleToggleConsole();
          break;

      case 73:   // 'I' key on keyboard
                 // 'INFO' key on remote
          this.handleGetInfo();
          break;

      default:
        console.log("GOT key code: " + k.keyCode)
          break;
    }

    return true;
  }

  static _states(){
    return [
          class IntroState extends this
          {
            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  IntroState");

              var dlg = this.tag("OkCancel");
              dlg.setSmooth('alpha', 0, {duration: 0.0});

              let h1 =  (1080 + 79); // Move LOWER blind to below bottom (offscreen)
              let h2 = -(h1/2 + 79); // Move UPPER blins to above top    (offscreen)

              const anim = this.tag('RDKlogo').animation(
              {
                duration: 0.5,  delay: 1.5,
                actions: [ { p: 'alpha', v: { 0: 1.0, 0.5: 0.75, 1: 0.0 } } ]
              });

              anim.on('finish', ()=>
              {
                this.tag('Blind1' ).setSmooth('y', h2, { delay: 0.25, duration: 0.75 });
                this.tag('Blind2' ).setSmooth('y', h1, { delay: 0.25, duration: 0.75 });

                this._setState('SetupState');
              });

              anim.start();
            }
          },
          class SetupState extends this
          {
            fetchAppList(url)
            {
              // Fetch App List
              //
              fetch(url)
              .then(res => res.json())
              .then((apps) =>
              {
                apps.map( (o) => o.pkgInstalled = false); //default

                AvailableApps = apps;
                InstalledApps = apps;

                this.tag("AvailableList").tiles = AvailableApps;
                this.tag("InstalledList").tiles = InstalledApps;

                this._setState('StoreRowState')
              })
              .catch(err =>
              {
                console.log("Failed to get URL: " + url);

                AvailableApps = DefaultApps;

                console.log("... using DefaultApps");

                this.tag("AvailableList").tiles = AvailableApps;

                this._setState('StoreRowState')
              });
            }
            fetchThunderCfg(url)
            {
              // Fetch Thunder Cfg
              //
              fetch(url)
              .then( res => res.json())
              .then((cfg) =>
              {
                console.log(' >>> Creating CUSTOM ThunderJS ...')
                thunderJS = ThunderJS(cfg);

                this.getInstalled();
              })
              .catch(err =>
              {
                console.log("Failed to get URL: " + url);

                console.log(' >>> Creating DEFAULT ThunderJS ...')
                thunderJS = ThunderJS(thunder_cfg);

                this.getInstalled();

                console.log("... using default Thunder cfg.");
              });
            }

            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  SetupState");

              const URL_PARAMS = new window.URLSearchParams(window.location.search)
              var appURL       = URL_PARAMS.get('appList')
              var cfgURL       = URL_PARAMS.get('thunderCfg')

              this.fetchThunderCfg(cfgURL);
              this.fetchAppList(appURL);

              // State advanced within 'fetchAppList()' above. 
            }
          },  //CLASS - SetupState
          // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
          class StoreRowState extends this
          {
            $enter()
            {
              // console.log(">>>>>>>>>>>>   STATE:  StoreRowState");

              // Set FOCUS to 1st package
              //
              var av_children = this.tag('AvailableList').children
              if(av_children.length >0)
              {
                av_children[this.storeButtonIndex].setFocus = true;
              }

              av_children.map( (o,n) => o.show(n * 0.15) );
            }

            _handleEnter()
            {
              let info   = AvailableApps[this.storeButtonIndex];
              let button = this.tag('AvailableList').children[this.storeButtonIndex];

              console.log("FIRE >>> INSTALL   pkgId:" + info.pkgId)

              button.fireAncestors('$InstallClicked', info.pkgId);

              var progress = button.tag("Progress")

              progress.reset(); // reset
              progress.setSmooth('alpha', 1, {duration: .1});
            }

            _handleDown()
            {
              this._setState('InstalledRowState');
            }

            _handleLeft()
            {
              if(--this.storeButtonIndex < 0) this.storeButtonIndex = 0;
            }

            _handleRight()
            {
              if(++this.storeButtonIndex > AvailableApps.length) this.storeButtonIndex = AvailableApps.length;
            }

            _getFocused()
            {
              return this.tag('AvailableList').children[this.storeButtonIndex]
            }
        }, //CLASS

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        class InstalledRowState extends this
        {
          $enter()
          {
            console.log(">>>>>>>>>>>>   STATE:  InstalledRowState");
          }

          _handleUp()
          {
            this._setState('StoreRowState');
          }

          _handleLeft()
          {
            if(--this.installedButtonIndex < 0) this.installedButtonIndex = 0;
          }

          _handleRight()
          {
            if(++this.installedButtonIndex > InstalledApps.length - 1) this.installedButtonIndex = InstalledApps.length - 1;
          }

          _handleEnter()
          {
            let info   = InstalledApps[this.installedButtonIndex];
            let button = this.tag('InstalledList').children[this.installedButtonIndex];

            console.log("FIRE >>> LAUNCH   pkgId:" + info.pkgId)

            button.fireAncestors('$LaunchClicked', info.pkgId);
            button.clickAnim();
          }

          _handleBack()
          {
            this._setState('OKCStateEnter')
          }

          _getFocused()
          {
            return this.tag('InstalledList').children[this.installedButtonIndex]
          }
        },//class
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        class OKCStateEnter extends this
        {
          $enter()
          {
            // console.log(">>>>>>>>>>>>   STATE:  OKCStateEnter");

            var button = this.tag('InstalledList').children[this.installedButtonIndex]

            if(button == undefined)
            {
              console.error(  'BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND')
              this.setConsole('BUTTON index:' + this.installedButtonIndex +'  - NOT FOUND');
              return;
            }
            var pkgId = button.info.pkgId;

            button.startWiggle();

            var dlg    = this.tag("OkCancel");
            dlg.pkgId  = pkgId; // needed later
            dlg.button = button;

            dlg.setLabel("Remove '" + pkgId + "' app ?");
            dlg.setSmooth('alpha', 1, {duration: 0.3});

            dlg._setState('OKCState')
          }

          _getFocused()
          {
            var dlg = this.tag("OkCancel");

            return dlg;
          }
        },
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      ]
  }//_states
}