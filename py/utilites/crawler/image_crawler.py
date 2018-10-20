from icrawler.builtin import BaiduImageCrawler
from icrawler.builtin import BingImageCrawler
from icrawler.builtin import GoogleImageCrawler
# from icrawler.builtin import FlickrImageCrawler
# import icrawler.builtin
import os

class Crawler:
    "crawler"
    def __init__(self, thread_parser=6, thread_downloader=9):
        self.thread_parser=thread_parser
        self.thread_downloader=thread_downloader
        self.keywords=[]
        self.dest_dir="./"
    def setKeywords(self, keywords):
        self.keywords=keywords
    def setDestDir(self, dir):
        self.dest_dir=dir

    def doGoogle(self):
        for keyword in self.keywords:
            google_storage = {'root_dir': '%s/google-%s/' % (self.dest_dir, keyword)}
            if os.path.exists(google_storage['root_dir']):
                continue
            google_crawler = GoogleImageCrawler(parser_threads=self.thread_parser,
                                                downloader_threads=self.thread_downloader,
                                                storage=google_storage)
            google_crawler.crawl(keyword=keyword,
                                 max_num=10000)
    def doBing(self):
        for keyword in self.keywords:
            bing_storage = {'root_dir': '%s/bing-%s/' % (self.dest_dir, keyword)}
            if os.path.exists(bing_storage['root_dir']):
                continue
            bing_crawler = BingImageCrawler(parser_threads=self.thread_parser,
                                            downloader_threads=self.thread_downloader,
                                            storage=bing_storage)
            bing_crawler.crawl(keyword=keyword,
                               max_num=100000)
    def doBaidu(self):
        for keyword in self.keywords:
            baidu_storage = {'root_dir': '%s/baidu-%s/' % (self.dest_dir, keyword)}
            if os.path.exists(baidu_storage['root_dir']):
                continue
            baidu_crawler = BaiduImageCrawler(parser_threads=self.thread_parser,
                                              downloader_threads=self.thread_downloader,
                                              storage=baidu_storage)
            baidu_crawler.crawl(keyword=keyword,
                                max_num=100000)
    def doAll(self):
        self.doBaidu()
        self.doBing()
        self.doGoogle()
