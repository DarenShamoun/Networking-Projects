echo "Beginning valid A request tests."
python3 resolver.py webopedia.com
python3 resolver.py pokemon.com
python3 resolver.py happycow.net
python3 resolver.py sdcoe.net
python3 resolver.py www.usd.edu
python3 resolver.py www.harvard.edu
python3 resolver.py consumerreports.org
python3 resolver.py pcta.org
python3 resolver.py github.io
python3 resolver.py gigatron.io
python3 resolver.py mentalhealth.gov
python3 resolver.py ca.gov
python3 resolver.py thecanadianencyclopedia.ca
python3 resolver.py sportsnet.ca

# the following require handling CNAME records
python3 resolver.py www.campuswire.com
python3 resolver.py en.wikipedia.org
python3 resolver.py catalogs.sandiego.edu

echo "Beginning MX request tests."
python3 resolver.py google.com --mx
python3 resolver.py drexel.edu --mx
python3 resolver.py archive.org --mx

echo "Beginning invalid A request tests."
python3 resolver.py floobatuba
python3 resolver.py devo.whipit
python3 resolver.py asflhasblkab.com
python3 resolver.py evil.sandiego.edu

echo "Beginning invalid MX request tests."
python3 resolver.py www.sandiego --mx
python3 resolver.py asflhasblkab.com --mx
