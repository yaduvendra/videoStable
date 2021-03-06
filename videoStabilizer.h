/**
 * @file     videoStabilizer.h
 * @brief    This class implements a video stabilization
 * @author   Mariano I. Lizarraga

  */

#ifndef VIDEOSTABILIZER_H
#define VIDEOSTABILIZER_H

#include <QObject>
#include <QImage>
#include <QBitArray>
#include <limits>
/**
@def    This defines determine the  the two bitplanes to be used in the
        Gray code calculation.
*/



#define SEARCH_FACTOR_P         9
#define HORIZ_WINDOW_M          25
#define VERT_WINDOW_N           25
#define PAN_FACTOR_D            0.75


/**
source: http://developer.gnome.org/glib/2.31/glib-Standard-Macros.html#MAX:CAPS
*/
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

class videoStabilizer : public QObject
{
    Q_OBJECT
public:
    /**
      This is the class constructor
    @param  videoSize           A Rect of the first frame in the video
    @return firstGrayCodeBit    The first in the two bits used for the gray code computation
    */
    videoStabilizer(QRect videoSize, QObject *parent = 0);

    ~videoStabilizer( );


signals:
    
public slots:
    void stabilizeImage(QImage* imageSrc, QImage* imageDest);

private:
    typedef QVector<QBitArray>  tGrayCodeMat;

    typedef QVector<uchar*> tImageMat;

    typedef struct _tcorrMatElement{
        int     m;
        int     n;
        uint    x;
        uint    y;
        uint      value;
    }tcorrMatElement;

    typedef struct _tSearchWindow{
        uint    lx;
        uint    ly;
        uint    rx;
        uint    ry;
    } tSearchWindow;


    /**
        This enumeration is used to determine which bit plane to employ in the GC calculations

        @enum BIT_PLANES
    */
    typedef enum _BIT_PLANES {
        GC_BP_0 = 1,
        GC_BP_1 = 2,
        GC_BP_2 = 4,
        GC_BP_3 = 8,
        GC_BP_4 = 16,
        GC_BP_5 = 32,
        GC_BP_6 = 64,
        GC_BP_7 = 128
    }BIT_PLANES;

    /**
       Computes the size of the four search windows (one for each subframe) as follows

       (lx,ly) -------------------------------------------------
               |       P                               P       |
               |<-P-> ----------------------------------- <-P->|
               |      |(m=0,n=0)                        |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |      |                                 |      |
               |<-P-> ----------------------------------- <-P->|
               |      P                                 P      |
               ------------------------------------------------- (rx,ry)

    */
    void computeSearchWindows ();

    /**
        Allocates the required memory and initializes all the data members
    */
    void allocateAndInitialize();

    /**
      Computes the first 9 indexes of the correlation location. These are the first 9
      indexes in the 3SS method.

      @param    subframe The subframe being used
      @param    seedX   X coordinate of UL point in the 9-point grid
      @param    seedY   Y coordinate of UL point in the 9-point grid
      @param    index   It indicates which is the start index for the corr mat
      @param    hFactor It indicates the horizontal spacing between sample points
      @param    vFactor It inidcates the vertical spacing between sample points
    */
    void computeCorrelationLocations(uint subframe, uint seedX, uint seedY, uint index, uint hFactor, uint vFactor);

    /**
      This function computes the graycode of an image
    @param  imageSrc        The full image for which the gray code will be computed
    @return imageGrayCode   The gray coded image
    */
    void getGrayCode();

    /**
        This function computes the gray code of each subframe's search window

    @param  subframe    The subframe for which to compute the gray code
    */
    void getSubframeGrayCode (uchar subframe, BIT_PLANES bitPlane = GC_BP_5);


    /**
      This function takes a QImage and scans line by line into uchar* to imageMatrix;

    @param  imageSrc        The image to be converted.
    @note   This function populates the imageMatrix data member.
    */
    void convertImageToMatrix(QImage* imageSrc);

    /**
        This function computes the Gray Code for a single byte

    @param  value       The 8bit value to be used in the Gray code calculation. The
                        bitplanes used are #defined in the header file.
    @param  bitPlane    The bitplane used to compute the graycode @see @enum BIT_PLANES
    */
    inline bool getByteGrayCode(uchar value, BIT_PLANES bitPlane = GC_BP_5);

    /**
        This function creates the de-rotated image to paint.

    @param  imageDest   The QImage where the final result will be painted on.
    */
    void populateImageResult(QImage* imageDest);


    /**
        This function computes the overall correlation of the subframes
    */
    void computeCorrelation();
    /**
        This function computes the Subframe correlation measures. It works over
    Gray coded images.

    @param  subframe    The subframe beinc computed
    @note   The correlation relies on some values #defined in the class' header
    */
    void computeSubframeCorrelation (uint index, uchar subframe, uchar t_m1);

    /**
        Compute single correlation for m,n offset;
    */
    inline void computeSingleCorrelation (uchar subframe, uchar t_m1, tcorrMatElement *element);


    /**
        This function uses each subframe's minimum and the last motion vector to compute
        the current motion vector.
    */
    void findMotionVector();

    /**
        This function sorts the subframe minima's and the last motion vector into an array

    @param      sortedMinima    The array where the five values will be sorted; Quicksort is used as the
                                sorting algorithm.
    @param      beg             The beginning index for sorting
    @param      end             The end index for sorting
    */
    void sortLocalMinima (tcorrMatElement* sortedMinima, char beg, char end);


    /**
        Helper function used to swap to tcorrMat elements
        @param      a,b         The elements to be swapped
    */
    inline void swap(tcorrMatElement* a, tcorrMatElement* b );


    /** Holds the height of the video */
    int videoHeight;
    /** Holds the width of the video */
    int videoWidth;
    /** Holds the current index of the grayCodeMatrix being used*/
    uchar currentGrayCodeIndex;
    /** Holds the size of the search window based on the search factor*/
    const uchar searchFactorWindow;
    /** Holds the vertical search offset for subframes LR and LL*/
    const uint vSearchOffset;
    /** Holds the horizontal search offset for subframes UR and LR*/
    const uint hSearchOffset;

    /** Holds the location of the ul and lr corners of each subframe*/
    tSearchWindow subframeLocations[4];

    /**
    This variable is an array of vectors that are in turn videoHeight arrays of QBitArrays that are
    videoWidth long. They take turns to hold g_k[t] and g_k[t-1]

    @see getGrayCode()
    */
    tGrayCodeMat grayCodeMatrix[2];

    /**
    This variable holds the image matrix currently being worked on.
    */
    tImageMat imageMatrix;

    /**
    This variable holds the 27 relevant values of the correlation matrix
    as defined in the 3SS method
    */
    tcorrMatElement correlationMatrix[4][18];

    /** This array holds the local minima of each subframe */
    tcorrMatElement localMinima[4];

    /** This element contains the motion vector at time t-1*/
    tcorrMatElement vg_tm1;

    /** This element contains the motion compensation vector at time t*/
    tcorrMatElement va;

    /** This element contains the motion compensation vector at time t-1*/
    tcorrMatElement va_tm1;

};

#endif // VIDEOSTABILIZER_H
