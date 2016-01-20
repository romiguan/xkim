#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

class TZException
{
public:
    enum ExceptCode
    {
        TZE_NOSERVICE = 1,

        TZE_NET_TIMEOUT,
        TZE_NET_PEERRESET,
        TZE_NET_OTHER,

        TZE_OPEN_SERVICE,
        TZE_APP,
        TZE_OTHER
    };

	TZException(int32_t tze_errno,int32_t tze_insight_no, const string &tze_what):
        tze_errno_(tze_errno),
        tze_insight_no_(tze_insight_no),
        tze_what_(tze_what)
	{
	}

	TZException(int32_t tze_errno,int32_t tze_insight_no):
        tze_errno_(tze_errno),
        tze_insight_no_(tze_insight_no)
	{
	}

	~TZException()
	{
	}

	int32_t tze_errno() const
	{
		return tze_errno_;
	};

	int32_t tze_insight_no() const
	{
		return tze_insight_no_;
	};

    const char *tze_what() const
    {
        return tze_what_.c_str();
    }

private:
	/**
	* Error NO. of exception,see @file defines.h
	*/
	int32_t tze_errno_;
	/**
	* Minor error NO. of exception
	*/
	int32_t tze_insight_no_;

    /**
     * exception description
     */
    string tze_what_;
};

#endif
