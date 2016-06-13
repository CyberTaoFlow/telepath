telepath.user = {
	
	showWindow: function () {
		
		Ext.define('MyApp.view.simpleuserWindow', {
			extend: 'Ext.window.Window',
			height: 310,
			width: 270,
			layout: { type: 'absolute' },
			title: 'User Editor',
			modal: true,
			initComponent: function() {
			
				var me = this;
				
				Ext.applyIf(me, {
					dockedItems: [
						{
							xtype: 'toolbar',
							dock: 'top',
							items: [{ xtype: 'button', text: 'Save User', listeners: { click: { fn: telepath.user.save,	scope: me } } }]
						}
					],
					items: [
						{ xtype: 'textfield',x: 10,y: 10, id: 's_user_email',fieldLabel: 'E-Mail' },
						{ xtype: 'textfield',x: 10,y: 40,id: 's_user_first_name',fieldLabel: 'First Name' },
						{ xtype: 'textfield',x: 10,y: 70,id: 's_user_last_name',fieldLabel: 'Last Name' },
						{ xtype: 'textfield',x: 10,y: 100,id: 's_user_company',fieldLabel: 'Company' },
						{ xtype: 'textfield',x: 10,y: 130,id: 's_user_phone',fieldLabel: 'Phone' },
						{ xtype: 'textfield',x: 10,y: 180,id: 's_user_password_1',fieldLabel: 'Password',inputType: 'password' },
						{ xtype: 'textfield',x: 10,y: 210,id: 's_user_password_2',fieldLabel: 'Password (again)',inputType: 'password' }
					]
				});
				
				me.callParent(arguments);
				
			}
		});
		
		telepath.user.editWindow = Ext.create('MyApp.view.simpleuserWindow');
		telepath.user.editWindow.show();
		telepath.user.getData();
	
	},
	getData: function() {
		
		telepath.util.request('/users/get_self', {}, function(data) {
			
			var userdata = data.items.user;
			
			Ext.getCmp('s_user_email').setValue(userdata.email);
			Ext.getCmp('s_user_first_name').setValue(userdata.first_name);
			Ext.getCmp('s_user_last_name').setValue(userdata.last_name);
			Ext.getCmp('s_user_company').setValue(userdata.company);
			Ext.getCmp('s_user_phone').setValue(userdata.phone);
			
		}, 'Error retreiving user information');
	
	},
	save: function () {
		
		var params = { };
		
		var fields = ['email', 'first_name', 'last_name', 'company', 'phone'];
		$.each(fields, function(i, field) {
			params[field] = Ext.getCmp('s_user_' + field).value;
		});
				
		// Validate email
		var email = params['email'];
		var email_regex = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
		if(email != '' && !email_regex.test(email)) {
			Ext.Msg.alert('Error', 'Incorrect email');
			return;
		}
		
		// Password change validate
		var pass_1 = Ext.getCmp('s_user_password_1').value;
		var pass_2 = Ext.getCmp('s_user_password_2').value;
		
		// Password mismatch
		if(pass_1 != '' && pass_2 != '' && pass_1 !== pass_2) {
			Ext.Msg.alert('Error', 'Password Mismatch');
			return;
		}
		
		// Password change
		if(pass_1 != '' && pass_2 != '' && pass_1 === pass_2) {
			params['password'] = pass_1;
		}
		
		telepath.user.user_mask = new Ext.LoadMask(telepath.user.editWindow.body.el, {msg:"Please wait..."});
		telepath.user.user_mask.show();
		
		$.post(telepath.controllerPath + '/users/set_self', params, function (data) {
			
			if(data.success) {
				telepath.user.editWindow.destroy();
			} else {
				telepath.user.user_mask.destroy();
				Ext.Msg.alert('Error', data.error);
			}
			
		}, 'json');
		
	}

}