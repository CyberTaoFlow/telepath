		
		try{
			if (finished==1)
				this.parent.Ext.getCmp('d_mapDidntLoad').hide();
		}
		catch(err) {
				this.parent.Ext.getCmp('d_mapDidntLoad').show();
		}

		//try {
		
			var components;
			// Set the authentication token, which is needed by the web-services to authenticate your application.
			ovi.mapsapi.util.ApplicationContext.set({"appId": "RK7D5aXdZ0ffPdfFPSRK", "authenticationToken": "3Wl9LeacmC5%2FRdgwWNYskA%3D%3D"});
			var mapContainer = document.getElementById("map");
				
				infobubbles = new ovi.mapsapi.map.component.InfoBubbles();
				//$('#mapContainer').height($this.parent);

				//one map containers are available in this example
				components = [
					infobubbles,
					new ovi.mapsapi.map.component.Behavior(),
					new ovi.mapsapi.map.component.TypeSelector(),
					new ovi.mapsapi.map.component.ZoomBar(),
					new ovi.mapsapi.map.component.ScaleBar(),
					new ovi.mapsapi.map.component.Overview(),
					new ovi.mapsapi.map.component.ViewControl(),
					new ovi.mapsapi.map.component.RightClick()
				];


				if (ovi.mapsapi.search.Manager) {
					components.push(new ovi.mapsapi.search.component.SearchComponent());
					components.push(new ovi.mapsapi.search.component.RightClick());
				}
				if (ovi.mapsapi.routing.Manager) {
					if (ovi.mapsapi.search.Manager)
						components.push(new ovi.mapsapi.routing.component.RouteComponent());
					components.push(new ovi.mapsapi.routing.component.RightClick());
				}
				//enterPoints(components);
				if (ovi.mapsapi.map && ovi.mapsapi.map.Display) {
					var map = (window.display = new ovi.mapsapi.map.Display(mapContainer, {
					components: components,
					zoomLevel: 2,
					fading: 250, // fading duration of tiles in miliseconds
					//center: [45.47, -85.58] //america
		//			center: [29.83,35.06]//eilat
					center: [20.9922, 12.58]//center of the world
					}));

				}
			
				//start_dashboard_timer(map);
				d_map = map
		//}
		//catch(err){
			
		//}