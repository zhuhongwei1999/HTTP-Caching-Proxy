FROM gcc:9

RUN mkdir /var/log/erss
RUN chmod +x /var/log/erss/run.sh
add . /var/log/erss/
WORKDIR /var/log/erss

ENTRYPOINT ["./run.sh"]