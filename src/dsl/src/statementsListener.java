// /*
//  * The PomPP software including libraries and tools is developed by the
//  * PomPP (Power Managemant Framework for Post-Petascale Supoercomputers)
//  * research project supported by the JST, CREST research program.
//  * The copyrights for code used under license from other parties are
//  * included in the corresponding files.
//  * 
//  * Copyright (c) 2017, The PomPP research team.
//  */

import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.misc.Interval;

import java.io.*;
import java.nio.file.*;
import java.nio.charset.*;
import java.util.*;
import java.text.*;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

public class statementsListener extends pompp_dslBaseListener {
	pompp_dslParser parser;
	public statementsListener(pompp_dslParser parser) { this.parser = parser; }
	
	@Override public void enterStatements(pompp_dslParser.StatementsContext ctx) { }
	
	@Override public void exitStatements(pompp_dslParser.StatementsContext ctx) { }
	
	@Override public void enterCreateStatement(pompp_dslParser.CreateStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(2).getText() + ".xml" );
			if ( objectFile.exists() ) { 
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(2).getText() + " is already created!" );
				return;
			}
			     	
			DocumentBuilder objectBuilder = null;
			objectBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
			Document objectDocument = objectBuilder.newDocument();
			
			Element objectRoot = objectDocument.createElement( ctx.getChild(2).getText() );
			objectDocument.appendChild( objectRoot );
                		
			Element objectType = objectDocument.createElement( "OBJECT_TYPE" );
			objectType.setTextContent( ctx.getChild(1).getText() );
			objectRoot.appendChild( objectType );
		                
			Transformer objectTransformer;
			TransformerFactory objectTransformerFactory = TransformerFactory.newInstance();
			objectTransformer = objectTransformerFactory.newTransformer();

			Result output = new StreamResult( new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(2).getText() + ".xml" ) );
			Source input = new DOMSource(objectDocument);
                		        
			objectTransformer.setOutputProperty(OutputKeys.INDENT, "yes");
			objectTransformer.transform(input, output);
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitCreateStatement(pompp_dslParser.CreateStatementContext ctx) { }
	
	@Override public void enterDeleteStatement(pompp_dslParser.DeleteStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { 
				objectFile.delete();
			}
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitDeleteStatement(pompp_dslParser.DeleteStatementContext ctx) { }
	
	@Override public void enterAddStatement(pompp_dslParser.AddStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			NodeList listOfProperties  = objectDocument.getElementsByTagName( ctx.getChild(2).getText() );
			
			if ( listOfProperties.getLength() == 0 ) {
				Node objectRoot = objectDocument.getElementsByTagName( ctx.getChild(1).getText() ).item(0);
				Element propertyToAdd = objectDocument.createElement( ctx.getChild(2).getText() );
				//propertyToAdd.setAttribute( "id", listOfProperties.getLength() + 1 );
				propertyToAdd.setTextContent( ctx.getChild(3).getText() );
				objectRoot.appendChild( propertyToAdd );

				TransformerFactory objectTransformerFactory = TransformerFactory.newInstance();
				Transformer objectTransformer = objectTransformerFactory.newTransformer();
				DOMSource objectDOMSource = new DOMSource( objectDocument );
				StreamResult objectStreamResult = new StreamResult( new File( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" ) );
				objectTransformer.transform( objectDOMSource, objectStreamResult );
			}

			else if ( listOfProperties.getLength() == 1 ) {
				System.out.println( "(" + ctx.getStop().getLine() + ") Property " + ctx.getChild(2).getText() + " already added!" );
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") More than one property are under the same name!" );
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitAddStatement(pompp_dslParser.AddStatementContext ctx) { }

	@Override public void enterRemoveStatement(pompp_dslParser.RemoveStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			NodeList listOfProperties  = objectDocument.getElementsByTagName( ctx.getChild(2).getText() );
			
			if ( listOfProperties.getLength() == 0 ) {
				System.out.println( "(" + ctx.getStop().getLine() + ") Property " + ctx.getChild(2).getText() + " does not exist!" );
			}

			else if ( listOfProperties.getLength() == 1 ) {
				Node objectRoot = objectDocument.getElementsByTagName( ctx.getChild(1).getText() ).item(0);
				Node propertyToRemove = listOfProperties.item( Integer.parseInt( ctx.getChild(3).getText() ) - 1 );					
				objectRoot.removeChild( propertyToRemove );
				
				TransformerFactory objectTransformerFactory = TransformerFactory.newInstance();
				Transformer objectTransformer = objectTransformerFactory.newTransformer();
				DOMSource objectDOMSource = new DOMSource( objectDocument );
				StreamResult objectStreamResult = new StreamResult( new File( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" ) );
				objectTransformer.transform( objectDOMSource, objectStreamResult );
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") More than one property are under the same name!" );
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitRemoveStatement(pompp_dslParser.RemoveStatementContext ctx) { }

	@Override public void enterListStatement(pompp_dslParser.ListStatementContext ctx) {
		try {
			File[] objectFiles = xmlFiles( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc" );
			
			if ( objectFiles.length > 0 ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") No object was found!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();

			for ( int i = 0; i < objectFiles.length; i++ ) {
				Document objectDocument = objectDocumentBuilder.parse( objectFiles[i] );
				objectDocument.getDocumentElement().normalize();
				Node objectType  = objectDocument.getElementsByTagName( "OBJECT_TYPE" ).item(0);				

				if ( ctx.getChild(1).getText().equals( "ALL" ) ) {
					System.out.println( "[ " + objectDocument.getDocumentElement().getNodeName() + ", " + objectType.getTextContent() + " ]" );
				}

				else {
					if ( objectType.getTextContent().equals( ctx.getChild(1).getText() ) ) {
						System.out.println( "[ " + objectDocument.getDocumentElement().getNodeName() + ", " + objectType.getTextContent() + " ]" );
					}
				}
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitListStatement(pompp_dslParser.ListStatementContext ctx) { }

	static File[] xmlFiles ( String directoryName ) {
		File directory = new File( directoryName );

		return directory.listFiles( new FilenameFilter() {
			public boolean accept( File directory, String filename )
			{ return filename.endsWith(".xml"); }
		} );
	}

	@Override public void enterGetStatement(pompp_dslParser.GetStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			if ( ctx.getChild(2).getText().equals( "ALL" ) ) {
				NodeList listOfProperties  = objectDocument.getElementsByTagName( ctx.getChild(1).getText() ).item(0).getChildNodes();
				
				for (int i = 0; i < listOfProperties.getLength(); i++) {
					Node propertyToGet = listOfProperties.item(i);

					if (propertyToGet.getNodeType() == Node.ELEMENT_NODE) {
						System.out.println( propertyToGet.getNodeName() + " = " + propertyToGet.getTextContent() );
					}
				}
			}

			else {
				Node propertyToGet  = objectDocument.getElementsByTagName( ctx.getChild(2).getText() ).item(0);
				System.out.println( propertyToGet.getNodeName() + " = " + propertyToGet.getTextContent() );
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitGetStatement(pompp_dslParser.GetStatementContext ctx) { }

	@Override public void enterSetStatement(pompp_dslParser.SetStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			NodeList listOfProperties  = objectDocument.getElementsByTagName( ctx.getChild(2).getText() );
			
			if ( listOfProperties.getLength() == 0 ) {
				System.out.println( "(" + ctx.getStop().getLine() + ") Property " + ctx.getChild(2).getText() + " does not exist!" );
			}

			else if ( listOfProperties.getLength() == 1 ) {
				Node propertyToSet = listOfProperties.item(0);
				propertyToSet.setTextContent( ctx.getChild(3).getText() );

				TransformerFactory objectTransformerFactory = TransformerFactory.newInstance();
				Transformer objectTransformer = objectTransformerFactory.newTransformer();
				DOMSource objectDOMSource = new DOMSource( objectDocument );
				StreamResult objectStreamResult = new StreamResult( new File( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" ) );
				objectTransformer.transform( objectDOMSource, objectStreamResult );
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") More than one property are under the same name!" );
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitSetStatement(pompp_dslParser.SetStatementContext ctx) { }

	@Override public void enterSwitchStatement(pompp_dslParser.SwitchStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			Node objectType  = objectDocument.getElementsByTagName( "OBJECT_TYPE" ).item(0);

			if ( objectType.getTextContent().equals( "MACHINE" ) ) { }

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " is not a MACHINE!" );
				return;
			}

			File machineConfFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/machine.conf" );
			
			if ( machineConfFile.exists() ) {
				Date now = new Date();
      				SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmsszzz");
				File machineConfFileBackUp = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/machine.conf." + dateFormat.format( now ) );
				machineConfFile.renameTo( machineConfFileBackUp );
			}

			FileWriter machineConfFileWriter = new FileWriter( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/machine.conf" );
			BufferedWriter machineConfBufferedWriter = new BufferedWriter( machineConfFileWriter );

			NodeList listOfProperties  = objectDocument.getElementsByTagName( ctx.getChild(1).getText() ).item(0).getChildNodes();
			
			for ( int i = 0; i < listOfProperties.getLength(); i++ ) {
				Node propertyToSwitch = listOfProperties.item(i);

				if ( propertyToSwitch.getNodeType() == Node.ELEMENT_NODE && !propertyToSwitch.getNodeName().equals ( "OBJECT_TYPE" ) ) {
					machineConfBufferedWriter.write ( propertyToSwitch.getNodeName() + "=" + propertyToSwitch.getTextContent() + "\n" );
				}
			}

			machineConfBufferedWriter.close();
			machineConfFileWriter.close();
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitSwitchStatement(pompp_dslParser.SwitchStatementContext ctx) { }

	@Override public void enterSubmitStatement(pompp_dslParser.SubmitStatementContext ctx) {
		try {
			File objectFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );
			
			if ( objectFile.exists() ) { }
			
			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " does not exist!" );
				return;
			}

			DocumentBuilderFactory objectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder objectDocumentBuilder = objectDocumentBuilderFactory.newDocumentBuilder();
			Document objectDocument = objectDocumentBuilder.parse( System.getenv( "POMPP_TOOLS_HOME" ).toString()+ "/etc/" + ctx.getChild(1).getText() + ".xml" );

			Node objectType  = objectDocument.getElementsByTagName( "OBJECT_TYPE" ).item(0);
			Node jobType  = objectDocument.getElementsByTagName( "JOB_TYPE" ).item(0);
			String numOfIterations = "", executablePath = "", numOfThreads = "", elapsedTime = "", PVTPath = "", optionsForExec = "", modulePower = "", controlMode = "", modelToUse = "", modelPath = "", profileName = "";

			if ( objectType.getTextContent().equals( "JOB" ) ) { }

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Object " + ctx.getChild(1).getText() + " is not a JOB!" );
				return;
			}

			if ( objectDocument.getElementsByTagName( "ITERATIONS" ).getLength() > 0 ) {
				numOfIterations  = objectDocument.getElementsByTagName( "ITERATIONS" ).item(0).getTextContent();
			}

			else {
				numOfIterations = "10";
			}

			if ( objectDocument.getElementsByTagName( "EXEC_PATH" ).getLength() > 0 ) {
				executablePath = objectDocument.getElementsByTagName( "EXEC_PATH" ).item(0).getTextContent();
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") JOB " + ctx.getChild(1).getText() + " does not have its EXEC_PATH specified!" );
				return;
			}

			if ( objectDocument.getElementsByTagName( "NUM_OF_THREADS" ).getLength() > 0 ) {
				numOfThreads = objectDocument.getElementsByTagName( "NUM_OF_THREADS" ).item(0).getTextContent();
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") JOB " + ctx.getChild(1).getText() + " does not have its NUM_OF_THREADS specified!" );
				return;
			}

			if ( objectDocument.getElementsByTagName( "ELAPSED_TIME" ).getLength() > 0 ) {
				elapsedTime = objectDocument.getElementsByTagName( "ELAPSED_TIME" ).item(0).getTextContent();
			}

			else {
				elapsedTime = "00:01:00";
			}

			if ( objectDocument.getElementsByTagName( "PVT_PATH" ).getLength() > 0 ) {
				PVTPath = objectDocument.getElementsByTagName( "PVT_PATH" ).item(0).getTextContent();
			}

			else {
				PVTPath = System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/pwr-spec-table.txt";
			}

			if ( objectDocument.getElementsByTagName( "OPTIONS" ).getLength() > 0 ) {
				optionsForExec = objectDocument.getElementsByTagName( "OPTIONS" ).item(0).getTextContent();
				optionsForExec = optionsForExec.substring( 1, optionsForExec.length() - 1 );
			}

			else {
				optionsForExec = "";
			}

			if ( jobType.getTextContent().equals( "OPTIMIZATION" ) ) { 
				if ( objectDocument.getElementsByTagName( "MODULE_POWER" ).getLength() > 0 ) {
					modulePower = objectDocument.getElementsByTagName( "MODULE_POWER" ).item(0).getTextContent();
				}

				else {
					System.out.println( "(" + ctx.getStop().getLine() + ") JOB " + ctx.getChild(1).getText() + " does not have its MODULE_POWER specified!" );
					return;
				}
			}

			if ( jobType.getTextContent().equals( "OPTIMIZATION" ) && objectDocument.getElementsByTagName( "CONTROL_MODE" ).getLength() > 0 ) {
				controlMode = objectDocument.getElementsByTagName( "CONTROL_MODE" ).item(0).getTextContent();
			}

			else {
				controlMode = "rapl";
			}
			
			if ( jobType.getTextContent().equals( "OPTIMIZATION" ) || jobType.getTextContent().equals( "PROFILING" ) ) {
				if ( objectDocument.getElementsByTagName( "PROFILE_NAME" ).getLength() > 0 ) {
					profileName = objectDocument.getElementsByTagName( "PROFILE_NAME" ).item(0).getTextContent();
				}
				
				else {
					System.out.println( "(" + ctx.getStop().getLine() + ") JOB " + ctx.getChild(1).getText() + " does not have its PROFILE_NAME specified!" );
					return;
				}
			}
			
			File modelFile;
			DocumentBuilderFactory modelObjectDocumentBuilderFactory;
			DocumentBuilder modelObjectDocumentBuilder;
			Document modelObjectDocument;

			Node modelObjectType;
			
			if ( jobType.getTextContent().equals( "OPTIMIZATION" ) && objectDocument.getElementsByTagName( "MODEL_TO_USE" ).getLength() > 0 ) {
				modelToUse = objectDocument.getElementsByTagName( "MODEL_TO_USE" ).item(0).getTextContent();
				
				modelFile = new File( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + modelToUse + ".xml" );
			
				if ( modelFile.exists() ) {
					modelObjectDocumentBuilderFactory = DocumentBuilderFactory.newInstance();
					modelObjectDocumentBuilder = modelObjectDocumentBuilderFactory.newDocumentBuilder();
					modelObjectDocument = modelObjectDocumentBuilder.parse( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/" + modelToUse + ".xml" );

					modelObjectType  = modelObjectDocument.getElementsByTagName( "OBJECT_TYPE" ).item(0);
					
					if ( modelObjectType.getTextContent().equals( "MODEL" ) ) {
						if ( modelObjectDocument.getElementsByTagName( "MODEL_PATH" ).getLength() > 0 ) {
							modelPath = modelObjectDocument.getElementsByTagName( "MODEL_PATH" ).item(0).getTextContent();
						}
						
						else {
							System.out.println( "(" + ctx.getStop().getLine() + ") Model " + modelToUse + " does not have its MODEL_PATH specified!" );
							return;
						}					
					}

					else {
						System.out.println( "(" + ctx.getStop().getLine() + ") Object " + modelToUse + " is not a MODEL!" );
						return;
					}				
				}
			
				else {
					System.out.println( "(" + ctx.getStop().getLine() + ") Model " + modelToUse + " does not exist!" );
					return;
				}
			}				

			FileWriter submissionScriptFileWriter = null;
			BufferedWriter submissionScriptBufferedWriter = null;
			Process jobToSubmit = null;

			Date now = new Date();
			SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMddHHmmsszzz");

			if ( jobType.getTextContent().equals( "CALIBRATION" ) ) {
				submissionScriptFileWriter = new FileWriter( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Calibration." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				submissionScriptBufferedWriter = new BufferedWriter( submissionScriptFileWriter );

				submissionScriptBufferedWriter.write( "#!/bin/bash\n" );
				submissionScriptBufferedWriter.write( "sh " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/bin/mpirun-ubmt.sh -itera " + numOfIterations + " -table " + PVTPath + " " + executablePath + " " + optionsForExec + "\n" );

				submissionScriptBufferedWriter.close();
				submissionScriptFileWriter.close();

				jobToSubmit = Runtime.getRuntime().exec( "chmod 777 " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Calibration." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				//jobToSubmit = Runtime.getRuntime().exec( "sh " System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Calibration." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );			
				jobToSubmit.waitFor();
			}

			else if ( jobType.getTextContent().equals( "GENERAL" ) ) {
				submissionScriptFileWriter = new FileWriter( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/General." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				submissionScriptBufferedWriter = new BufferedWriter( submissionScriptFileWriter );

    				submissionScriptBufferedWriter.write( "#!/bin/bash\n" );
			    	submissionScriptBufferedWriter.write( "#PJM -L \"vnode=" + numOfThreads + "\"\n" );
			    	submissionScriptBufferedWriter.write( "#PJM -L \"vnode-core=1\"\n" );
			    	submissionScriptBufferedWriter.write( "#PJM -L \"elapse=" + elapsedTime + "\"\n" );
			    	submissionScriptBufferedWriter.write( "\n" );
			    	submissionScriptBufferedWriter.write( executablePath + " " + optionsForExec + "\n" );

				submissionScriptBufferedWriter.close();
				submissionScriptFileWriter.close();

				jobToSubmit = Runtime.getRuntime().exec( "chmod 777 " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/General." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				//jobToSubmit = Runtime.getRuntime().exec( "pjsub " System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/General." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );			
				jobToSubmit.waitFor();
			}

			else if ( jobType.getTextContent().equals( "PROFILING" ) ) {
				submissionScriptFileWriter = new FileWriter( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Profiling." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				submissionScriptBufferedWriter = new BufferedWriter( submissionScriptFileWriter );

				submissionScriptBufferedWriter.write( "#!/bin/bash\n" );
				submissionScriptBufferedWriter.write( "sh " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/bin/mpirun-prof.sh -app " + ctx.getChild(1).getText() + " -itera " + numOfIterations + " -table " + PVTPath + " " + executablePath + " " + optionsForExec + "\n" );

				submissionScriptBufferedWriter.close();
				submissionScriptFileWriter.close();

				jobToSubmit = Runtime.getRuntime().exec( "chmod 777 " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Profiling." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				//jobToSubmit = Runtime.getRuntime().exec( "sh " System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Profiling." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );			
				jobToSubmit.waitFor();
			}

			else if ( jobType.getTextContent().equals( "OPTIMIZATION" ) ) {
				submissionScriptFileWriter = new FileWriter( System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Optimization." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				submissionScriptBufferedWriter = new BufferedWriter( submissionScriptFileWriter );

				submissionScriptBufferedWriter.write( "#!/bin/bash\n" );
				submissionScriptBufferedWriter.write( "sh " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/bin/mpirun-opt.sh -power " + modulePower + " -prof no -app " + profileName + " -mode " + controlMode + " -model " + modelPath + " "
				+ executablePath + " " + optionsForExec + "\n" );

				submissionScriptBufferedWriter.close();
				submissionScriptFileWriter.close();

				jobToSubmit = Runtime.getRuntime().exec( "chmod 777 " + System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Optimization." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );
				//jobToSubmit = Runtime.getRuntime().exec( "sh " System.getenv("POMPP_TOOLS_HOME").toString()+ "/etc/Optimized." + ctx.getChild(1).getText() + "." + dateFormat.format( now ) + ".sh" );			
				jobToSubmit.waitFor();
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") JOB " + ctx.getChild(1).getText() + " does not have a valid JOB_TYPE!" );
				return;
			}
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitSubmitStatement(pompp_dslParser.SubmitStatementContext ctx) { }

	@Override public void enterInsertStatement(pompp_dslParser.InsertStatementContext ctx) {
		String POMPP_Argument = "";

		if ( ctx.getChild(4).getText().equals("POMPP_Start_Sec") || ctx.getChild(4).getText().equals("POMPP_Stop_Sec") ) {
			POMPP_Argument = ctx.getChild(5).getText();
		}

		String POMPP_Init_C = "\tPOMPP_Init(argc, argv);";
		String POMPP_Finalize_C = "\tPOMPP_Finalize();";
		String POMPP_Start_Sec_C = "\tPOMPP_Start_section(\"".concat( POMPP_Argument ).concat( "\");" );
		String POMPP_Stop_Sec_C = "\tPOMPP_Stop_section(\"".concat( POMPP_Argument ).concat( "\");" );

		String POMPP_Init_F = "      call POMPP_Init()";
		String POMPP_Finalize_F = "      call POMPP_Finalize()";
		String POMPP_Start_Sec_F = "      call POMPP_Start_section(\"".concat( POMPP_Argument ).concat( "\")" );
		String POMPP_Stop_Sec_F = "      call POMPP_Stop_section(\"".concat( POMPP_Argument ).concat( "\")" );

		String toInsert = "";  

		try {
			Path srcPath = Paths.get( ctx.getChild(1).getText() );
			List<String> srcInLines = Files.readAllLines(srcPath, StandardCharsets.UTF_8);

			int lineNumber = Integer.parseInt( ctx.getChild(2).getText() ) - 1;

			if ( ctx.getChild(4).getText().equals("POMPP_Init") && ctx.getChild(3).getText().equals("C") ) {
				toInsert = POMPP_Init_C;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Finalize") && ctx.getChild(3).getText().equals("C") ) {
				toInsert = POMPP_Finalize_C;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Start_Sec") && ctx.getChild(3).getText().equals("C") ) {
				toInsert = POMPP_Start_Sec_C;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Stop_Sec") && ctx.getChild(3).getText().equals("C") ) {
				toInsert = POMPP_Stop_Sec_C;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Init") && ctx.getChild(3).getText().equals("Fortran") ) {
				toInsert = POMPP_Init_F;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Finalize") && ctx.getChild(3).getText().equals("Fortran") ) {
				toInsert = POMPP_Finalize_F;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Start_Sec") && ctx.getChild(3).getText().equals("Fortran") ) {
				toInsert = POMPP_Start_Sec_F;
			}

			else if ( ctx.getChild(4).getText().equals("POMPP_Stop_Sec") && ctx.getChild(3).getText().equals("Fortran") ) {
				toInsert = POMPP_Stop_Sec_F;
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Invalid statement!" );
				return;
			}

			srcInLines.add( lineNumber, toInsert );
			Files.write( srcPath, srcInLines, StandardCharsets.UTF_8 );
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitInsertStatement(pompp_dslParser.InsertStatementContext ctx) { }

	@Override public void enterCommentOutStatement(pompp_dslParser.CommentOutStatementContext ctx) {
		String C_Comment = "// ";
		String F_Comment = "! ";
		String commentedLine = "";

		try {
			Path srcPath = Paths.get( ctx.getChild(1).getText() );
			List<String> srcInLines = Files.readAllLines(srcPath, StandardCharsets.UTF_8);

			int lineNumber = Integer.parseInt( ctx.getChild(2).getText() ) - 1;

			if ( ctx.getChild(3).getText().equals("C") ) {
				commentedLine = "\t// ".concat( srcInLines.get( lineNumber ).trim() );
			}

			else if ( ctx.getChild(3).getText().equals("Fortran") ) {
				commentedLine = "      ! ".concat( srcInLines.get( lineNumber ).trim() );
			}

			else {
				System.out.println( "(" + ctx.getStop().getLine() + ") Invalid statement!" );
				return;
			}

			srcInLines.set( lineNumber, commentedLine );
			Files.write( srcPath, srcInLines, StandardCharsets.UTF_8 );
		}

		catch (Exception e) {
			System.err.println( e.toString() );
		}
	}

	@Override public void exitCommentOutStatement(pompp_dslParser.CommentOutStatementContext ctx) { }
}
			
