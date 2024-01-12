#include "ImageProcessor.h"
#include "Constants.h"
#include "ImageConstructor.h"
#include "ImageConstructorFactory.h"

#include<string>
#include<thread>
#include<format>
#include<chrono>

using std::stringstream;
using std::this_thread::get_id;
using std::format;
using std::chrono::system_clock;

using cv::imwrite;
using cv::imshow;
using cv::Vec2b;
using cv::Vec3b;
using cv::Vec4b;
using cv::waitKey;
using cv::destroyWindow;
using cv::namedWindow;
using cv::WINDOW_KEEPRATIO;

using std::to_string;

ImageProcessor::ImageProcessor()
{
	msg_logger_->LogDebug("Image processor default constructor.");
	image_ = Mat(1, 1, CV_8UC1);
}

ImageProcessor::ImageProcessor(Mat image)
{
	image_ = image;
}

/*
Parameterized constructor; constructs an image having imageDimensions from the given imageDataMap.
*/
ImageProcessor::ImageProcessor(map<unsigned short, std::string> image_data_map, const Size& image_dimensions, const uint& image_file_size)
{
	short no_of_channels = image_file_size / (image_dimensions.width * image_dimensions.height);
	msg_logger_->LogDebug("Inside ImageProcessor. Number of channels: " + to_string(no_of_channels));

	ImageConstructor* image_constructor = ImageConstructorFactory::GetImageConstructor(no_of_channels, image_data_map, image_dimensions);
	if (image_constructor == nullptr) {
		msg_logger_->LogError("ERROR: Constructing image with " + to_string(no_of_channels) + "is currently not supported.");
		return;
	}

	image_ = image_constructor->ConstructImage();
}

ImageProcessor::~ImageProcessor()
{
	msg_logger_->LogDebug("Image processor destructor.");
}

void ImageProcessor::DisplayImage(cv::String window_name)
{
	namedWindow(window_name, WINDOW_KEEPRATIO);
	imshow(window_name, image_);

	cv::waitKey(0);
	cv::destroyWindow(window_name);
}

/*
This functions saves the modified image at the location of the original image.
*/
void ImageProcessor::SaveImage(std::string original_image_address)
{
	bool was_image_written = imwrite(GetAddressToSaveModifiedImage(original_image_address), image_);
	if (!was_image_written) {
		msg_logger_->LogError("ERROR: Image could not be written to file.");
		return;
	}
	msg_logger_->LogDebug("Image written to file successfully.");
}

Mat ImageProcessor::GetImage()
{
	return image_;
}

/*
This function constructs the address to save the modified image.
The new address is the same as that of original image, with the suffix '_modified_<timestamp>' added.
*/
std::string ImageProcessor::GetAddressToSaveModifiedImage(std::string original_image_address) 
{
	//Below snippet to convert chrono::time_point to string taken from https://stackoverflow.com/a/46240575
	string current_time_string = format("{:%H%M%S}", system_clock::now());

	ushort dot_index = original_image_address.find_last_of('.');
	string modified_image_save_address = string(original_image_address, 0, dot_index)
		.append(MODIFIED_SUFFIX).append(UNDERSCORE).append(current_time_string)
		.append(original_image_address, dot_index, original_image_address.length());

	return modified_image_save_address;
}

void ImageProcessor::DisplayOriginalAndFilteredImage(const Mat& original_image, const Mat& filtered_image)
{
	//Below snippet to convert thread id to string taken from https://stackoverflow.com/a/19255203
	stringstream s_stream;
	s_stream << get_id();

	string original_image_window_name = ORIGINAL_IMAGE_WINDOW_NAME + s_stream.str();
	string filtered_image_window_name = FILTERED_IMAGE_WINDOW_NAME + s_stream.str();

	namedWindow(original_image_window_name, cv::WINDOW_AUTOSIZE);
	imshow(original_image_window_name, original_image);

	namedWindow(filtered_image_window_name, cv::WINDOW_AUTOSIZE);
	imshow(filtered_image_window_name, filtered_image);

	waitKey(0);
	destroyWindow(original_image_window_name);
	destroyWindow(filtered_image_window_name);
}

