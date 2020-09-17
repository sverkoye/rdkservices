import { Utils } from 'wpe-lightning-sdk'

import Progress from "./Progress";

export default class AppTile extends lng.Component {
    static _template( )
    {
      var pts = 20;
      var barClr1  = 0xFF888888;  // #888888FF  // Background
      var frameClr = 0xFF666666;  // #666666FF
      var textClr  = 0xFFffffff;  // #ffffffFF

      var stroke    = 2;
      var strokeClr = 0xFF444444;

      let RR = { radius: 20, type: lng.shaders.RoundedRectangle, stroke: stroke, strokeColor: strokeClr}

      return {
        flexItem: { margin: 40 },
        alpha: 1.0,
        Button:
        {
          scale: 0.0,

          RRect:
          {
              w: 210, h: 150, rtt: true, rect: true, pivot: 0.5, alpha: 1.0, color: frameClr, shader: RR,

              Image: {
                mountX: 0.5,
                mountY: 0.5,
                src: Utils.asset('images/crate2_80x80.png'),
                x: (w => 0.45 * w),
                y: (h => 0.4 * h)
              },

              Label:
              { mountX: 0.5, mountY: 1.0, x: (w => 0.5 * w), y: (h => h - 5),
                text: {  text: "Label 11", fontFace: 'Regular', fontSize: pts, textColor: textClr,

                shadow: true,
                shadowColor: 0xFF000000,
                shadowOffsetX: 2,
                shadowOffsetY: 2,
                shadowBlur: 8,
                },
              },

              Icon: {
                alpha: 1.0,
                mountX: 1.0,
                scale: 0.52,
                x: (w => w + 4),
                y: (h => 0)
              },
            },

            Progress: { type: Progress, mountX: 0.0, x: 0, y: 190, w: 210, h: 8, alpha: 0.0 },
        }//Button
      }
    };

    setInfo(i)
    {
    //  console.log('setInfo() ... this.info >>> ' + JSON.stringify(i) );

      if(i.name)  this.setLabel(i.name)
      else
      if(i.label) this.setLabel(i.label)
      else
      if(i.pkgId) this.setLabel(i.pkgId)
      else        this.setLabel("unknown22")

      var check_mark_PNG = Utils.asset('images/check_mark.png');
      var download_PNG   = Utils.asset('images/download3.png');

      var icon = (i.pkgInstalled) ? check_mark_PNG : download_PNG;
      this.setIcon(icon);
    }

    setLabel(s)
    {
      var obj = this.tag("Label")
      obj.text.text = s;
    }

    setIcon(s)
    {
      var btn = this.tag("Button");
      var icn = btn.tag("Icon")

      icn.patch( {src: s } );
    }

    _focus()
    {
      var btn = this.tag("Button");
      var bg  = btn.tag("RRect")

      console.log("BUTTON:  pkgId: " + this.pkgId);

      bg.setSmooth('alpha', 1.00, {duration: 0.3});
      bg.setSmooth('scale', 1.15, {duration: 0.3});
    }

    _unfocus()
    {
      var btn = this.tag("Button");
      var bg  = btn.tag("RRect")

      // bg.setSmooth('alpha', 0.5, {duration: 0.3});
      bg.setSmooth('scale', 1.0, {duration: 0.3});
    }

    hide()
    {
      if(this.tag("Button").scale == 1.0)
      {
        const anim = this.tag('Button').animation({
          duration: 0.5,
          actions: [
              { p: 'scale', v: { 0: 1, 0.5: 0.50, 1: 0.0 } },
          ]
        });
        anim.start();
      }
    }

    show(d = 0)
    {
      if(this.tag("Button").scale == 0.0)
      {
        const anim = this.tag('Button').animation({
          duration: 0.5,
          delay: d,
          actions: [
              { p: 'scale', v: { 0: 0, 0.5: 0.50, 1: 1.0 } },
          ]
        });
        anim.start();
      }
    }

    _init()
    {
      // console.log('AppTile() ... this.info >>> ' + JSON.stringify(this.info) );

      // console.log('AppTile() ... this.info.showTile >>> ' + this.info.showTile );
      // this.tag("Button").alpha = this.info.showTile;

      this._installed = false;
      this.tag("Button").scale = 0;

      if(this.w && this.h)
      {
        let button = this.tag("Button");

        button.w = this.w;
        button.h = this.h;
      }

      // var check_mark_PNG = Utils.asset('images/check_mark.png');
      // var download_PNG   = Utils.asset('images/download3.png');

      // let installed = this.info.installed;

      // this.setInstalled( installed )
      // this.setIcon(installed ? check_mark_PNG : download_PNG)

      this.tileInfo = this.info;
    }

    set tileInfo( ii )
    {
      if(ii == undefined || ii == null)
      {
        return
      }

      if(ii.pkgId)
      {
        this.pkgId = ii.pkgId
        //console.log('tileInfo() ... this.pkgId: ' + this.pkgId );
      }

      this.setInfo(ii);
    }

    startWiggle()
    {
        this.wiggleAnim = this.tag('Image').animation({
            duration: 0.11*3,
            repeat: -1,
            actions: [
              {
                t: '',
                repeat: -1,
                p: 'rotation', v: { 0: 0.0, 0.25: -0.14, 0.5: 0.0, 0.75: 0.14, 1: 0.0},
              },
            ],
          });

        this.wiggleAnim.start()
    }

    stopWiggle()
    {
      this.wiggleAnim.stop();
      this.wiggleAnim = null;
    }

    setInstalled(v)
    {
      this._installed = v;
    }

    isInstalled()
    {
      return this._installed;
    }

  }//CLASS
