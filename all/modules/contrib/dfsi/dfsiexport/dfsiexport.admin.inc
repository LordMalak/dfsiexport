<?php

// ------------------------------------------------
// @(#) dfsiexport.admin.inc
// @(#) Configuration du mode d'export
// @(#) Amael Hervoche
// @(#) 21/01/2014
// ------------------------------------------------


// ------------------------------------------------
// Hook_settings
// ------------------------------------------------
function dfsiexport_admin_settings()
{
    $dfsiexport_preferred_exportMode=variable_get("dfsiexport_preferred_exportMode", 1 );
    
	$form['modeExport'] = array
	(
		'#type' => 'radios',
		'#title' => t("Mode d'exportation"),
		'#options' => array
		(
			_DFSIEXPORT_MODE_EXPORT_CSV =>t('Csv'),
			_DFSIEXPORT_MODE_EXPORT_XML =>t('Xml'),
		),
		'#default_value' => variable_get('modeExport','#options'),
	);      
	$form['#submit'][] = 'dfsiexport_settings_form_submit';
	return system_settings_form($form);	
}



function dfsiexport_settings_form_submit($form, &$form_state) 
{    
    $dfsiexport_choosed_mode = $form_state['values']['modeExport'];
	variable_set('modeExport',$dfsiexport_choosed_mode);
	return $dfsiexport_choosed_mode;
}