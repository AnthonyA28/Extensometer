// opencv 
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp> // Lucas Kanade
#include <opencv2/imgproc.hpp>// Drawing shapes
// basic includes
#include <iostream>
// Create paths
#include <filesystem>
// Create files
#include <fstream>
namespace fs = std::filesystem;
// power 
#include <math.h> 

using namespace cv;
using namespace std;

Mat image;
string path;
string imgType; 

Point p1(0, 0);
Point p2(0, 0);
Point p3(0, 0);
Point p4(0, 0);

vector<string> imgVec; 
vector<float> distVec; 

void checkForImageMapping()
{
    vector<float> imgTimes;

    string dataPath = path.substr(0, path.find_last_of("\\"));
    string line;
    ifstream mapFile(dataPath + "\\ImageMapping.csv");
    if (mapFile.is_open())
    {
        bool okayToParse = false; 
        while (getline(mapFile, line))
        {
            int commaPos = line.find_first_of(",");
            if (commaPos != -1) {
                string colOne = line.substr(0, commaPos);
                if (colOne == "Time") {
                    okayToParse = true;
                    continue; 
                }
                if (okayToParse)
                {
                    imgTimes.push_back(stoi(colOne));
                }
            }
        }
        mapFile.close();
    }
    else {
        cout << "Could not find ImageMapping.csv\n";
        return;
    }

    vector<float> measurementTimes;
    vector<string> otherData; 
    ifstream rawDataFile(dataPath + "\\RawData.csv" );
    if (rawDataFile.is_open())
    {
        bool okayToParse = false;
        while (getline(rawDataFile, line))
        {
            int commaPos = line.find_first_of(",");
            if (commaPos != -1) {
                string colOne = line.substr(0, commaPos);
                if (colOne == "(ms)") {
                    okayToParse = true; 
                    continue; 
                }
                if (okayToParse) 
                {
                    measurementTimes.push_back(stoi(colOne));
                    otherData.push_back(line);

                }
            }
        }
        rawDataFile.close();
    }
    else {
        cout << "Could not find RawData.csv\n";
        return;
    }

    ofstream resultFile(fs::path(dataPath + "\\Results_sync.csv"), ofstream::trunc);
    resultFile << "Strain [%] (DIC), Time(img), Time (measured), Extenso, Force, Displacement\n";
    assert(distVec.size() == imgTimes.size());
    for (int i = 0; i < imgTimes.size(); i++)
    {
        int index = lower_bound(measurementTimes.begin(), measurementTimes.end(), imgTimes[i]) - measurementTimes.begin();
        if (index < otherData.size()) {
            resultFile << (distVec[i]-distVec[0])/ distVec[0]*100.0 << ", " << imgTimes[i] << ", " << otherData[index] << "\n";
        }
    }

    resultFile.close();

}


void track()
{
    cout << path << "\n";

    cout << "Getting list of images with type " << imgType << "\n";
    for (const auto& entry : fs::directory_iterator(path)) {
        
        string p = entry.path().generic_string(); 
        if (p.ends_with(imgType)) {

            imgVec.push_back(p.substr(p.find_last_of("/") + 1));
            std::cout << p << std::endl;
        }
    }

    struct {
        bool operator()(string a, string b) const { 
            a = a.substr(a.find_last_of("/") + 1);
            b = b.substr(b.find_last_of("/") + 1);
            a = a.substr(0, a.find_last_of("."));
            b = b.substr(0, b.find_last_of("."));
            int a_int = stoi(a); 
            int b_int = stoi(b);
            return a_int < b_int; 
        }
    } customLess;
    

    std::sort(imgVec.begin(), imgVec.end(), customLess);

    cout << "Number of images to analyze: " << imgVec.size() << "\n";


    vector<uchar> status;
    vector<float> err;
    TermCriteria criteria = TermCriteria((TermCriteria::COUNT)+(TermCriteria::EPS), 10, 0.03);
    vector<Point2f> points0, points1;



    if (p4.x < p3.x)
    {
        int x = p4.x;
        p4.x = p3.x;
        p3.x = x;
    }
    if (p2.x < p1.x)
    {
        int x = p2.x;
        p2.x = p1.x;
        p1.x = x;
    }
    if (p2.y < p1.y)
    {
        int y = p2.y;
        p2.y = p1.y;
        p1.y = y;
    }
    if (p4.y < p3.y)
    {
        int y = p4.y;
        p4.y = p3.y;
        p3.y = y;
    }
    int width  = ((p4.x-p3.x) + (p2.x-p1.x)) / 2;
    int height = ((p4.y-p3.y) + (p2.y-p1.y)) / 2;
    cout << "width: " << width << '\n';
    cout << "height: " << height << '\n';

    Point init1;
    init1.x = (p1.x + p2.x) / 2;
    init1.y = (p1.y + p2.y) / 2;

    Point init2;
    init2.x = (p3.x + p4.x) / 2;
    init2.y = (p3.y + p4.y) / 2;
    points0.push_back(init1);
    points0.push_back(init2);


    // Creating a directory
    fs::path savePath = fs::path(path.substr(0, path.find_last_of("\\") ) + "\\DIC");
    if (fs::create_directory(savePath))
    {
        cout << "Made the save directory at " << savePath << "\n";
    }
    else
    {
        cout << "Failed to create output path at " << savePath << "\n";
        //return;
    }

    ofstream resultFile(fs::path(savePath.generic_string() + "Results.csv"), ofstream::trunc);
    resultFile << "Distance, filename\n";
    float distance = sqrt(pow((points0[1].x - points0[0].x), 2) + pow((points0[1].x - points0[0].x), 2));
    distVec.push_back(distance); 
    resultFile << distance << ", " << imgVec[0] << "\n";
    
    for (int i = 0; i < imgVec.size()-1; i += 1)
    {


        Mat icur = imread(path + "\\" + imgVec[i], IMREAD_COLOR);
        Mat inext = imread(path + "\\" + imgVec[i+1], IMREAD_COLOR);

        calcOpticalFlowPyrLK(icur, inext, points0, points1, status, err, Size(width, height), 2, criteria);


        Point rp1; 
        Point rp2;
        rp1.x = points1[0].x - width / 2;
        rp2.x = points1[0].x + width / 2;
        rp1.y = points1[0].y - height / 2;
        rp2.y = points1[0].y + height / 2;
        rectangle(icur, rp1, rp2,
            Scalar(255, 0, 0),
            2, LINE_8);
        rp1.x = points1[1].x - width / 2;
        rp2.x = points1[1].x + width / 2;
        rp1.y = points1[1].y - height / 2;
        rp2.y = points1[1].y + height / 2;
        rectangle(icur, rp1, rp2,
            Scalar(0, 0, 255),
            2, LINE_8);

        //imshow("DispWin", icur); // Show our image inside it.

        cout << "i: " << i << "\n";

        distance = sqrt(pow((points1[1].x - points1[0].x), 2) + pow((points1[1].x - points1[0].x), 2));
        distVec.push_back(distance);
        //imgVec[i + 1].substr(imgVec[i + 1].find_last_of("\\"), imgVec[i + 1].length() - imgVec[i + 1].find_last_of("\\")) 
        resultFile << distance << ", " << imgVec[i + 1] << "\n";
        resultFile.flush(); 
        

        string save_name = savePath.generic_string() + "/" + imgVec[i + 1];
        //string save_name = imgVec[i + 1].substr(0, imgVec[i + 1].find_last_of(".")) + "_DIC.tif";
        cout << "Saving to " << save_name << "\n";
        imwrite(save_name, icur);
        points0[0].x = points1[0].x;
        points0[0].y = points1[0].y;
        points0[1].x = points1[1].x;
        points0[1].y = points1[1].y;
    }

    resultFile.close();

    checkForImageMapping();

}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        
        p1.x = x; 
        p1.y = y; 
    }
    else if (event == EVENT_LBUTTONUP)
    {
        p2.x = x;
        p2.y = y;
        cout << p1.x << ", " << p1.y << ", " << "  " << p2.x << ", " << p2.y << "\n";

        Mat cloned = image.clone();
        rectangle(cloned, p1, p2,
            Scalar(255, 0, 0),
            2, LINE_8);
        rectangle(cloned, p3, p4,
            Scalar(0, 0, 255),
            2, LINE_8);

        imshow("DispWin", cloned); // Show our image inside it.
    }
    if (event == EVENT_RBUTTONDOWN)
    {

        p3.x = x;
        p3.y = y;
    }
    else if (event == EVENT_RBUTTONUP)
    {
        p4.x = x;
        p4.y = y;
        
        Mat cloned = image.clone();
        rectangle(cloned, p1, p2,
            Scalar(255, 0, 0),
            2, LINE_8);
        rectangle(cloned, p3, p4,
            Scalar(0, 0, 255),
            2, LINE_8);

        imshow("DispWin", cloned); // Show our image inside it.
    }
    else if (event == EVENT_MBUTTONDOWN)
    {
        track();
    }

}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cout << "Reference Image required \n\tUsage: " << argv[0] << " <Reference Image>" << endl;
        return -1;
    }
    
    //cout << argv[1];
    path = argv[1];
    imgType = path.substr(path.find_last_of("."), path.length() - path.find_last_of("."));
    cout << "image type : " << imgType << "\n";
    path = path.substr(0, path.find_last_of("\\"));
    

    image = imread(argv[1], IMREAD_COLOR); // Read the file
    if (image.empty()) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    namedWindow("DispWin", WINDOW_AUTOSIZE); // Create a window for display.
    setMouseCallback("DispWin", CallBackFunc, NULL);
    imshow("DispWin", image); // Show our image inside it.
    waitKey(0); // Wait for a keystroke in the window
    return 0;
}
