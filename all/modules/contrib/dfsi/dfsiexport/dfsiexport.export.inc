<?php
// ------------------------------------------------
// @(#) dfsiexport.export.inc
// @(#) export des donnees
// @(#) Amael Hervoche
// @(#) 24/01/2014
// ------------------------------------------------



// ---------------------------------------
// callback ajax 
// appelé sur changement d'etat des cases
// à cocher
// export dependances implique export noeuds
// ---------------------------------------
function dfsiexport_Ajax_CheckBoxesOptionsCallback( $form, &$form_state ) 
{
	if( $form['dfsiexport_choixExport'][2]['#checked'] == true )
	{
		$form['dfsiexport_choixExport'][1]['#checked'] = true;
	}
	return $form['dfsiexport_choixExport'];
}

// ---------------------------------------
// renvoie le tableau associatif des options
// possibles d'éléments à exporter
// ---------------------------------------
function dfsiexport_ExportFormGetOptions()
{
	return array
	(
		1 => t('Containers et services'),
		2 => t('Dépendances (directes et indirectes)'),
		3 => t('Taxonomies (types de service et constructeurs)')	
	);
}

// ---------------------------------------
// formulaire exportation
// ---------------------------------------
function dfsiexport_ExportForm( $form, &$form_state ) 
{
	$form['dfsiexport_choixExport'] = array
	(
		'#type' => 'checkboxes',
		'#title' => t("Eléments à exporter"),
		'#title_display'  => 'before',
		'#options' => dfsiexport_ExportFormGetOptions(),
		'#ajax' => array
		(
			'callback' => 'dfsiexport_Ajax_CheckBoxesOptionsCallback',
			'wrapper' => 'CheckBoxesOptions',       // nom de la div à reafficher en Ajax
			'event' => 'change',                    // event declencheur
			'method' => 'replace',                  // que faire de la div
			//'effect' => 'slide',                  // effet visuel
			'progress' => array('type' => 'none'),  // pas de barre de progression
		),
		'#value' => array(),
		'#attributes' => array(),
		'#prefix' => '<div id="CheckBoxesOptions">',
		'#suffix' => '</div>',
	);

	$form['dfsiexport_choixExport'] = form_process_checkboxes($form['dfsiexport_choixExport']);

	$form['submit'] = array
	(
		'#type' => 'submit',
		'#value' => t('Exporter'),
		'#weight' => 255,
		'#submit' => array('dfsiexport_ExportForm_form_submit', 'dfsiexport_GetStructuredDataToExport'),
	);
	return  $form;
}

// ---------------------------------------
// Génère le nom de fichier pour
// l'exportation
// ---------------------------------------
function dfsiexport_CreateExportFileName()
{
	$result = date("Ymd-h-m-s")."-"."dfsi"."-export.txt";
	return $result;
}

// ---------------------------------------
// sauvegarer les type de données à exporter
// cochées
// ---------------------------------------
function dfsiexport_SaveExportOptions($form)
{
	$choix_export = array(); 
	// mise à jour des types d'export preferes

	for ($i = 1; $i <= 3; $i++) 
	{
		if( $form['dfsiexport_choixExport'][$i]['#checked'] == true )
		{
			$choix_export[] = $i;
		}
	}
	variable_set('dfsiexport_choixExport', $choix_export ); 

	return $choix_export;
}

// ---------------------------------------
// lecture des données à exporter
// ---------------------------------------
function dfsiexport_GetDataToExport()
{
	$result = array();

	$query = new EntityFieldQuery();
	$query
		->entityCondition('entity_type', 'node')
		->entityCondition('bundle', array(_DFSI_SERVICE,_DFSI_CONTAINER))
		->propertyCondition('status', 1);

	$queryResult = $query->execute();
	if (isset($queryResult['node'])) 
	{
		$nids = array_keys($queryResult['node']);
		$result = entity_load('node', $nids);
	}

	return $result;
}

// ---------------------------------------
// transformation des données brutes
// en fonction du type d'exportation
// ---------------------------------------
function dfsiexport_GetStructuredDataToExport($ModeExport, $ExportData, $form)
{
	$result = "";

	switch($ModeExport )
	{
		case _DFSIEXPORT_MODE_EXPORT_CSV:
			$result = dfsiexport_CSV($ModeExport, $ExportData, $form);
		break;
		case _DFSIEXPORT_MODE_EXPORT_XML:
			$result = dfsiexport_XML($ModeExport, $ExportData, $form);
		break;
	}

    return $result;
}

// ---------------------------------------
// retourne les données d'un champ pour l'export CSV.
// ---------------------------------------
function dfsiexport_GetExportText_csv( $fieldName , $node )
{
	$result = "";

	$wrapper = entity_metadata_wrapper('node', $node);  
	$fieldValue = $wrapper-> $fieldName-> value();

	// -----------------------------
	// La valeur est de type tableau
	// -----------------------------
	if(  gettype ($fieldValue) == 'array' )
	{
		// -------------------------------------
		// c'est un champ de type node reference
		// -------------------------------------
		if( !isset($fieldValue['value']) )
		{
			$refList = '"';
			foreach($fieldValue as $oneReference )
			{
				$refList .=   $oneReference->nid;
				$refList .=  ', ';
			}

			// retrait virgule finale;
			$refList = rtrim( $refList, ", " );
			$refList .= '"';
			$result .= $refList.', ';
		}
	}

	return $result;
}

// ----------------------------------------
// Fonction export CSV
// ----------------------------------------
function dfsiexport_CSV($ModeExport, $ExportData, $form)
{
	$result = "";

	foreach($ExportData as $key =>  $oneNode )
	{
		$result .= dfsiexport_get_node_csv($oneNode, $key);										// node -> Toujours

		if( $form['dfsiexport_choixExport'][2]['#checked'] == true )
        {
			$result .= dfsiexport_get_dependance_csv($oneNode);									// dépendance -> seulement si deuxième case cochée
		}
		// retrait virgule finale;
		$result = rtrim($result, ", ");
		$result .="\r\n";
	}
	// Taxonomie -------------------------------------------------------------------------- // A faire
	if( $form['dfsiexport_choixExport'][3]['#checked'] == true )
	{
		$result .= dfsiexport_get_taxo_csv();
	}

	return $result;
}

// Export de noeuds
function dfsiexport_get_node_csv($oneNode, $key)
{
	$result = "";

	// NID
	$result .= "{$key}, ";
	// type de noeud
	$result .= '"'.dfsi_GetTypeOf($oneNode).'", ';
	// titre
	$result .= '"'.dfsi_GetTitleOf($oneNode).'", ';
	// Statut
	$result .= '"'.dfsiexport_GetStatutsOf($oneNode).'", ';
	// Description (body)
	$result .= '"'.dfsiexport_GetBodyOf($oneNode).'", ';
	// Criticite
	$result .= '"'.dfsiexport_GetCriticiteOf($oneNode).'", ';
	// taxonomie
	$result .= '"'.$oneNode->field_service_type['und'][0]['tid'].'", ';

	return $result;
}

// Export des dependances entre noeuds
function dfsiexport_get_dependance_csv($oneNode)
{
	$result = "";

	// recuperation du nom des champs
	$fields = field_info_instances('node', dfsi_GetTypeOf($oneNode));

	// --------------------
	// pour tous les champs
	// --------------------
	foreach( $fields as $key => $field )
	{
		// recuperation de la valeur du champ pour ce noeud
		$fieldName = $field['field_name'];
		$result .= dfsiexport_GetExportText_csv( $fieldName , $oneNode );
	}

	return $result;
}

// Export taxonomies 
function dfsiexport_get_taxo_csv()
{
	$result = "";

	// fonction drupal qui retourne un tableau d'objet vocabulaire
	$vocabularies = taxonomy_get_vocabularies();

	// récupère catégorie de taxonomie
	foreach ($vocabularies as $vocab)
	{
		$result .= "\r\n";
		$result .= $vocab->name;
		$result .= "\r\n";
		
		// Cree une representation hierarchise du vocabulaire
		$terms = taxonomy_get_tree($vocab->vid,  0, NULL, 1);

		foreach($terms as $term)
		{
			// Term
			$result .= "\"$term->tid:$term->name\"";
			$result .= ", ";
		}
	}

	return $result;
}


// ----------------------------------------
// Fonction export XML
// ----------------------------------------
function dfsiexport_XML($ModeExport, $ExportData, $form)
{
	$result = "";

	$result .= dfsiexport_xml_version();

	$result .= dfsiexport_xml_schema();

	$result .= "<dfsi>";
	$result .= "\r\n";

	// Noeud
	$result .= "\t<services>";
	$result .= "\r\n";
	foreach($ExportData as $key =>  $oneNode )
	{
		if(dfsi_GetTypeOf($oneNode) == "itservice")
		{
				$result .= dfsiexport_get_node_serv_xml($oneNode, $key);
		}
	}
	$result .= "\t</services>";
	$result .= "\r\n";
	
	$result .= "\t<containers>";
	$result .= "\r\n";
	foreach($ExportData as $key =>  $oneNode )
	{
		if(dfsi_GetTypeOf($oneNode) == "itcontainer")
		{
			$result .= dfsiexport_get_node_cont_xml($oneNode, $key);
		}
	}
	$result .= "\t</containers>";
	$result .= "\r\n";
	
	// Taxonomie
	if( $form['dfsiexport_choixExport'][3]['#checked'] == true )
	{
		$result .= dfsiexport_get_taxo_xml();
	}
	$result .= "</dfsi>";

	return $result;
}

// ---------------------------------------
// retourne les données d'un champ pour l'export XML.
// ---------------------------------------
function dfsiexport_GetExportText_xml( $fieldName , $node )
{
	$result = "";

	$wrapper = entity_metadata_wrapper('node', $node);  
	$fieldValue = $wrapper-> $fieldName-> value();

	// -----------------------------
	// La valeur est de type tableau
	// -----------------------------
	if(  gettype ($fieldValue) == 'array' )
	{
		// cas du champ body
		if( !isset($fieldValue['value']) )
		{
			// -------------------------------------
			// c'est un champ de type node reference
			// -------------------------------------
			$refList = "";
			foreach($fieldValue as $oneReference )
			{
				$refList = "\t\t\t\t<$fieldName>$oneReference->nid</$fieldName>";
				$refList .= "\r\n";
			}
			$del = array("field_" => "");
			$refList = strtr($refList, $del);
			$result .= $refList;
		}
	}

	return $result;
}

// Numero de version du XML au début du fichier
function dfsiexport_xml_version()
{
	$result = "";

	$result .= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	$result .= "\r\n\r\n";

	return $result;
}

// Schema au debut fichier
function dfsiexport_xml_schema()
{
	$result = "";

	$result .= "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";
	$result .= "\r\n";
	$result .= "\t<xs:element name=\"dfsi\">";
	$result .= "\r\n";
	$result .= "\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t<xs:element name=\"services\">";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"service\">";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"numero\" type=\"xs:integer\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"titre\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"statut\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"body\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"criticite\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"taxonomie\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t</xs:element>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t</xs:element>";
	$result .= "\r\n";
	$result .= "\t\t\t\t<xs:element name=\"containers\">";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"container\">";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"numero\" type=\"xs:integer\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"titre\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"statut\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"body\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"criticite\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"taxonomie\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t</xs:element>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t</xs:element>";
	$result .= "\r\n";
	$result .= "\t\t\t\t<xs:element name=\"taxonomies\">";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t<xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t<xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"categorie\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t\t<xs:element maxOccurs=\"unbounded\" name=\"terme\" type=\"xs:string\"/>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t\t\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t\t\t\t</xs:element>";
	$result .= "\r\n";
	$result .= "\t\t\t</xs:sequence>";
	$result .= "\r\n";
	$result .= "\t\t</xs:complexType>";
	$result .= "\r\n";
	$result .= "\t</xs:element>";
	$result .= "\r\n";
	$result .= "</xs:schema>";
	$result .= "\r\n\r\n";
	
	return $result;
}

// Exportation des noeuds de type itservice
function dfsiexport_get_node_serv_xml($oneNode, $key)
{
	$result = "";

	$result .= "\t\t<service>";
	$result .= "\r\n";
	$result .= "\t\t\t<numero>" . $key . "</numero>";
	$result .= "\r\n";
	$result .= "\t\t\t<titre>" . dfsi_GetTitleOf($oneNode) . "</titre>";
	$result .= "\r\n";
	$result .= "\t\t\t<statut>" . dfsiexport_GetStatutsOf($oneNode) . "</statut>";
	$result .= "\r\n";
	$result .= "\t\t\t<body>" . dfsiexport_GetBodyOf($oneNode) . "</body>";
	$result .= "\r\n";
	$result .= "\t\t\t<criticite>" . dfsiexport_GetCriticiteOf($oneNode) . "</criticite>";
	$result .= "\r\n";
	$result .= "\t\t\t<taxonomie>" . $oneNode->field_service_type['und'][0]['tid'] . "</taxonomie>";
	$result .= "\r\n";
	$result .= "\t\t\t<dependances>";
	$result .= "\r\n";
	$result .= dfsiexport_get_dependance_xml($oneNode);
	$result .= "\t\t\t</dependances>";
	$result .= "\r\n";
	$result .= "\t\t</service>";
	$result .= "\r\n";

	return $result;
}

// Exportation des noeuds de type itcontainer
function dfsiexport_get_node_cont_xml($oneNode, $key)
{
	$result = "";

	$result .= "\t\t<container>";
	$result .= "\r\n";
	$result .= "\t\t\t<numero>" . $key . "</numero>";
	$result .= "\r\n";
	$result .= "\t\t\t<titre>" . dfsi_GetTitleOf($oneNode) . "</titre>";
	$result .= "\r\n";
	$result .= "\t\t\t<statut>" . dfsiexport_GetStatutsOf($oneNode) . "</statut>";
	$result .= "\r\n";
	$result .= "\t\t\t<body>" . dfsiexport_GetBodyOf($oneNode) . "</body>";
	$result .= "\r\n";
	$result .= "\t\t\t<criticite>" . dfsiexport_GetCriticiteOf($oneNode) . "</criticite>";
	$result .= "\r\n";
	$result .= "\t\t\t<taxonomie>" . $oneNode->field_service_type['und'][0]['tid'] . "</taxonomie>";
	$result .= "\r\n";
	$result .= "\t\t\t<dependances>";
	$result .= "\r\n";
	$result .= dfsiexport_get_dependance_xml($oneNode);
	$result .= "\t\t\t</dependances>";
	$result .= "\r\n";
	$result .= "\t\t</container>";
	$result .= "\r\n";

	return $result;
}

// Export dependances entre noeuds
function dfsiexport_get_dependance_xml($oneNode)
{
	$result = "";

	// recuperation du nom des champs
	$fields = field_info_instances('node', dfsi_GetTypeOf($oneNode));

	// --------------------
	// pour tous les champs
	// --------------------
	foreach( $fields as $key => $field )
	{
		// recuperation de la valeur du champ pour ce noeud
		$fieldName = $field['field_name'];
		$result .= dfsiexport_GetExportText_xml( $fieldName , $oneNode );
	}
	
	return $result;
}

// Export taxonomie
function dfsiexport_get_taxo_xml()
{
	$result = "";

	$result .= "\t<taxonomies>";
	$result .= "\r\n";

	// fonction drupal qui retourne un tableau d'objet vocabulaire
	$vocabularies = taxonomy_get_vocabularies();

	// renvoi la catégorie du vocabulaire
	foreach ($vocabularies as $vocab)
	{
		$result .= "\t\t<categorie>";
		$result .= "\r\n";
		$result .= "\t\t\t".$vocab->name;
		$result .= "\r\n";
		$result .= "\t\t</categories>";
		$result .= "\r\n";

		// Cree une representation hierarchise du vocabulaire
		$terms = taxonomy_get_tree($vocab->vid,  0, NULL, 1);

		foreach($terms as $term)
		{
			// Term
			$result .= "\t\t\t<terme id=\"$term->tid\">";
			$result .= "\r\n";
			$result .= "\t\t\t\t\"".$term->name."\"";
			$result .= "\r\n";
			$result .= "\t\t\t</terme>";
			$result .= "\r\n";
		}
	}

	$result .= "\t</taxonomies>";
	$result .= "\r\n";

	return $result;
}

// ---------------------------------------
// traitement du submit export
// ---------------------------------------
function dfsiexport_ExportForm_form_submit($form, &$form_state) 
{
       
	$dfsiexport_DataChoiceToExport = dfsiexport_SaveExportOptions($form);
   
  
   // lecture des données brutes
	$dfsiexport_ExportData = dfsiexport_GetDataToExport();     
   

	$dfsiexport_ModeExport = variable_get('modeExport',_DFSIEXPORT_MODE_EXPORT_CSV);
	
	$dfsiexport_choix_export = dfsiexport_SaveExportOptions($form);
	
	// transformation de données brutes lues en données csv ou Xml
	$dfsiexport_StructuredExportData = dfsiexport_GetStructuredDataToExport($dfsiexport_ModeExport, $dfsiexport_ExportData, $form);
	
	// construction du nom et chemin de ficheir d'export
	$dfsiexport_fileName = dfsiexport_CreateExportFileName();
	$dfsexport_ExportDestination = "temporary://{$dfsiexport_fileName}";  
	
	// sauvegarde du fichier d'export
	$filePath = file_unmanaged_save_data($dfsiexport_StructuredExportData, $dfsexport_ExportDestination, $replace = FILE_EXISTS_REPLACE);
	
	// download navigateur
	file_transfer
    (
        $filePath, array
        (
            'Content-Type' => 'text/csv',
            'Content-Disposition' => 'attachment; fileName="'.$dfsiexport_fileName.'"'
        )      
    );
	return $dfsiexport_ModeExport;
    // rien ne sera exécuté ici car appel à drupal_exit(),  pas moyen d'effecer le fichier temporaire ici !!!!
}
// ---------------------------------------
// validation du formulaire d'export
// rien à faire dans l'immédiat...
// ---------------------------------------
function dfsiexport_ExportForm_form_validate($form, &$form_state) 
{
    
}
    
// ---------------------------------------
// ---------------------------------------
function dfsiexport_Import()
{
    $output = "";
    $output = array
    (
	'contentImport' => array
        (
            '#markup' => 'Import de données',
        ),
    );
	return $output;
}