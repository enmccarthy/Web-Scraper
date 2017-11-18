#include <stdio.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>

#include <curl/curl.h>

uint write_data(char *ptr, uint size, uint nmemb, TidyBuffer *stream)
{
    uint r;
    r = size * nmemb;
    printf("the size of the buffer is %ui", r);
    tidyBufAppend(stream, ptr, r);
    return r;
}

void dumpNode(TidyDoc doc, TidyNode tnod, int indent, FILE * pagefile)
{
    TidyNode child;
    
    for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
      ctmbstr name = tidyNodeGetName(child);
      if(name) {
	/* if it has a name, then it's an HTML tag ... */
	TidyAttr attr;
	fprintf(pagefile, "%*.*s%s ", indent, indent, "<", name);
	/* walk the attribute list */
	for(attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
	  fprintf(pagefile, tidyAttrName(attr));
	  tidyAttrValue(attr)?fprintf(pagefile,"=\"%s\" ",
				      tidyAttrValue(attr)):fprintf(pagefile," ");
	}
	fprintf(pagefile,">\n");
      }
      else {
	/* if it doesn't have a name, then it's probably text, cdata, etc... */
	TidyBuffer buf;
	tidyBufInit(&buf);
	tidyNodeGetText(doc, child, &buf);
	fprintf(pagefile, "%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
	tidyBufFree(&buf);
      }
      
      dumpNode(doc, child, indent + 4, pagefile); /* recursive */
    }
}

int main(int argc, char *argv[])
{
    CURL *curl_handle;
    char curl_errbuf[CURL_ERROR_SIZE];
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;
 
    if(argc < 2) {
        printf("Usage: %s <URL>\n", argv[0]);
        return 1;
    }
    //char * name = argv[0];
    //char *pagefilename = strcat(".out");

    char *pagefilename = "web.out";
    /* init the curl session */ 
    curl_handle = curl_easy_init();
 
    /* set URL to get here */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
 
    /* Switch on full protocol/debug output while testing */ 
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
 
    /* disable progress meter, set to 0L to enable and disable debug output */ 
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    
    // curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION,1);

    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(tdoc, &tidy_errbuf);
    tidyBufInit(&docbuf);
 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &docbuf);
    err=curl_easy_perform(curl_handle);
    if(!err) {
      err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
      if(err >= 0) {
	err = tidyCleanAndRepair(tdoc); /* fix any problems */
	if(err >= 0) {
	  err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
	  if(err >= 0) {
	     /* open the file */
	    FILE *pagefile;
	    pagefile = fopen(pagefilename, "wb");
	    dumpNode(tdoc, tidyGetRoot(tdoc), 0, pagefile); /* walk the tree */
	    fclose(pagefile);
	    fprintf(stderr, "%s\n", tidy_errbuf.bp); /* show errors */
	  }
	}
      }
    }
    else
      fprintf(stderr, "%s\n", curl_errbuf);

    /* clean-up */
    curl_easy_cleanup(curl_handle);
    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);
    return err;
    
}
