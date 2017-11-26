#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>
#include <pylon/usb/BaslerUsbInstantCameraArray.h>

#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include<windows.h>
#include <stdlib.h>
#include <stdio.h>
// Use sstream to create image names including integer
#include <sstream>
#if defined( USE_USB )
// Settings to use Basler USB cameras.
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef CBaslerUsbCameraEventHandler CameraEventHandler_t; // Or use Camera_t::CameraEventHandler_t
using namespace Basler_UsbCameraParams;
#endif
// Namespace for using pylon objects.
using namespace Pylon;
using namespace Basler_UsbCameraParams;
// Namespace for using GenApi objects
using namespace GenApi;

// Namespace for using opencv objects.
using namespace cv;

// Namespace for using cout.
using namespace std;

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 100;
// Number of cameras.
static const size_t c_maxCamerasToUse = 2;



int main(int argc, char* argv[])
{
	// The exit code of the sample application
	int exitCode = 0;

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;

	// Create an example event handler. In the present case, we use one single camera handler for handling multiple camera events.
	// The handler prints a message for each received event.

	// Create another more generic event handler printing out information about the node for which an event callback
	// is fired.

	try
	{

		// Create an instant camera object with the first found camera device matching the specified device class.
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();
		// Get all attached devices and exit application if no device is found.
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			throw RUNTIME_EXCEPTION("No camera present.");
		}
		// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
		CBaslerUsbInstantCameraArray cameras(2);
		// Create and attach all Pylon Devices.
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

			// Print the model name of the camera.
			cout << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
		}
		// Open the camera for setting parameters.
		cameras.Open();
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Width.SetValue(320);
			cameras[i].Height.SetValue(240);
			// Select OPIO line 1
			cameras[i].LineSelector.SetValue(LineSelector_Line1);
			// Set the line mode to Input
			cameras[i].LineMode.SetValue(LineMode_Input);
			// Get the current line mode
			// Set camera for hardware Frame Burst Start Trigger (code from Ace USB Users Manual)
			cameras[i].AcquisitionMode.SetValue(AcquisitionMode_Continuous);
			cameras[i].TriggerSelector.SetValue(TriggerSelector_FrameBurstStart);
			cameras[i].TriggerMode.SetValue(TriggerMode_Off);
			cameras[i].TriggerSelector.SetValue(TriggerSelector_FrameStart);
			cameras[i].TriggerMode.SetValue(TriggerMode_On);
			cameras[i].AcquisitionFrameRateEnable.SetValue(false);
			cameras[i].TriggerSource.SetValue(TriggerSource_Line1);
			cameras[i].TriggerActivation.SetValue(TriggerActivation_RisingEdge);
			cameras[i].ExposureMode.SetValue(ExposureMode_Timed);
			cameras[i].ExposureTime.SetValue(20000);
			cameras[i].TriggerDelay.SetValue(0);
			cameras[i].SensorReadoutMode.SetValue(SensorReadoutMode_Fast);
		}
		// create pylon image format converter and pylon image
		CImageFormatConverter formatConverter0;
		formatConverter0.OutputPixelFormat = PixelType_BGR8packed;
		CPylonImage pylonImage0;
		CImageFormatConverter formatConverter1;
		formatConverter1.OutputPixelFormat = PixelType_BGR8packed;
		CPylonImage pylonImage1;
		// Create an OpenCV image
		Mat openCvImage1;
		Mat openCvImage0;
		// Create an Directory
		LPCWSTR szDirPath0 = L"C:/Users/mango/Desktop/c++/leftimages";
		CreateDirectory(szDirPath0, NULL);
		LPCWSTR szDirPath1 = L"C:/Users/mango/Desktop/c++/rightimages";
		CreateDirectory(szDirPath1, NULL);
		cameras[1].AcquisitionStart.Execute(); // MJR: Don't think this is necessary. Called by camera.StartGrabbing()?
		cameras[0].AcquisitionStart.Execute();
		int grabbedImages1 = 0;
		int grabbedImages0 = 0;
		cameras[1].StartGrabbing(c_countOfImagesToGrab, GrabStrategy_OneByOne, GrabLoop_ProvidedByUser);
		cameras[0].StartGrabbing(c_countOfImagesToGrab, GrabStrategy_OneByOne, GrabLoop_ProvidedByUser);
		CGrabResultPtr ptrGrabResult1;
		CGrabResultPtr ptrGrabResult0;
		cout << endl << "You have 5s to start trigger" << endl;
		while (cameras[0].IsGrabbing() || cameras[1].IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 5 s is used.
			
			cameras[0].RetrieveResult(5000, ptrGrabResult0, TimeoutHandling_ThrowException);
			cameras[1].RetrieveResult(5000, ptrGrabResult1, TimeoutHandling_ThrowException);
			// Image grabbed successfully?
			
				if (ptrGrabResult0->GrabSucceeded())
				{
					// Access the image data.
					cout << "SizeX: " << ptrGrabResult0->GetWidth() << endl;
					cout << "SizeY: " << ptrGrabResult0->GetHeight() << endl;
					const uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult0->GetBuffer();
					cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;

					// Convert the grabbed buffer to pylon imag
					formatConverter0.Convert(pylonImage0, ptrGrabResult0);
					// Create an OpenCV image out of pylon image
					openCvImage0 = cv::Mat(ptrGrabResult0->GetHeight(), ptrGrabResult0->GetWidth(), CV_8UC3, (uint8_t *)pylonImage0.GetBuffer());

					// Create a display window
					namedWindow("leftimages", CV_WINDOW_NORMAL);//AUTOSIZE //FREERATIO
																		   // Display the current image with opencv
					imshow("leftimages", openCvImage0);
					std::ostringstream s;
					s << "C:/Users/mango/Desktop/c++/leftimages/leftimage_" << grabbedImages0 << ".jpg";
					std::string imageName(s.str());
					imwrite(imageName, openCvImage0);
					grabbedImages0++;
					// Define a timeout for customer's input in ms.
					// '0' means indefinite, i.e. the next image will be displayed after closing the window 
					// '1' means live stream
					waitKey(1);

				}
				else
				{
					cout << "Error: " << ptrGrabResult0->GetErrorCode() << " " << ptrGrabResult0->GetErrorDescription() << endl;
				}
				if (ptrGrabResult1->GrabSucceeded())
				{
					// Access the image data.
					cout << "SizeX: " << ptrGrabResult1->GetWidth() << endl;
					cout << "SizeY: " << ptrGrabResult1->GetHeight() << endl;
					const uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult1->GetBuffer();
					cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;

					// Convert the grabbed buffer to pylon imag
					formatConverter1.Convert(pylonImage1, ptrGrabResult1);
					// Create an OpenCV image out of pylon image
					openCvImage1 = cv::Mat(ptrGrabResult1->GetHeight(), ptrGrabResult1->GetWidth(), CV_8UC3, (uint8_t *)pylonImage1.GetBuffer());

					// Create a display window
					namedWindow("rightimages", CV_WINDOW_NORMAL);//AUTOSIZE //FREERATIO
																		   // Display the current image with opencv
					imshow("rightimages", openCvImage1);
					std::ostringstream s;
					s << "C:/Users/mango/Desktop/c++/rightimages/rightimage_" << grabbedImages1 << ".jpg";
					std::string imageName(s.str());
					imwrite(imageName, openCvImage1);
					grabbedImages1++;
					// Define a timeout for customer's input in ms.
					// '0' means indefinite, i.e. the next image will be displayed after closing the window 
					// '1' means live stream
					waitKey(1);

				}
				else
				{
					cout << "Error: " << ptrGrabResult1->GetErrorCode() << " " << ptrGrabResult1->GetErrorDescription() << endl;
				}
			
		}


		cameras[0].AcquisitionStop.Execute(); // MJR: Don't think this is necessary
		cameras[1].AcquisitionStop.Execute();
										  // while ( camera.IsGrabbing() && cnt < 10000)

										  //camera.StopGrabbing();              // MJR: Don't think this is necessary
		cameras.Close();
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}


	// // Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return exitCode;
}