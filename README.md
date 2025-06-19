# Time-Sensitive-Networking

## Preduslovi za simulaciju

*Potrebno je poduzeti sljedeće korake kako bi se instalirala potrebna verzija ns-3 simulatora, kao i tsn dodaci:*

- *Preuzmite ns-3 simulator i instalirajte ga:*

```bash
cd ~;
curl -LO https://www.nsnam.org/releases/ns-allinone-3.31.tar.bz2;
tar -xvf ns-allinone-3.31.tar.bz2;
rm ns-allinone-3.31.tar.bz2;
rm ns-allinone-3.31/ns-3.31/contrib/ -rf && cp Time-Aware-Shaper-TAS-in-ns-3/ns-3_Implementation/contrib/ ns-allinone-3.31/ns-3.31/contrib/ -r;
git clone https://github.com/DenKrysos/Time-Aware-Shaper-TAS-in-ns-3.git;
cd ns-allinone-3.31/ns-3.31/;
sudo apt install python2 -y;
sed -i 's/python3/python2/g' waf;
./waf configure --enable-tests --enable-examples --disable-python && ./waf build;
```

*Kod se može pokrenuti na sljedeći način:*

```bash
./waf --run=TSN
```
*ili ako je već instaliran ns3 koji koristi verziju 3.42, kopirati TSN.cc u "scratch" folder i pokrenuti iz direktorija /ns-allinone-3.42/ns-3.42$:*
```bash
./ns3 run scratch/TSN.cc 
```
